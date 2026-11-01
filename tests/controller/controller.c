/* Execute the real linked Z80 instructions against a scripted SNES PIO. */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hw/z80.h"
#include "utils/vtimer.h"

static z80 cpu;
static device_t ram, pio;
static unsigned char memory[65536], image[65536];
static unsigned short headers[2];
static unsigned char axes[2][2], speeds[2];
static unsigned index_bit, pins, cycles, latches, samples, steps;
static int sensitivity, clone, current_image;
struct event { unsigned long t; unsigned index, value; char kind; };
static struct event events[512];
static unsigned nevents;
static void event(char kind, unsigned value) {
    assert(nevents < 512);
    events[nevents++] = (struct event){cpu.cyc,index_bit,value,kind};
}
static unsigned char mem_read(device_t *dev, unsigned address) {
    (void)dev; return memory[address & 65535];
}
static void mem_write(device_t *dev, unsigned address, unsigned char value) {
    (void)dev; memory[address & 65535] = value;
}
static unsigned char io_read(device_t *dev, unsigned address) {
    (void)dev; assert(address == 0);
    unsigned value = 0;
    for (unsigned p=0; p<2; p++) {
        unsigned bit;
        if (index_bit < 16) {
            unsigned h = headers[p];
            if (sensitivity) h = (h & ~0xc00) | (speeds[p]==1 ? 0x800 : speeds[p]==2 ? 0x400 : 0);
            bit = (h >> index_bit) & 1;
        } else if (index_bit < 32) {
            bit = (axes[p][(index_bit-16)/8] >> (7-(index_bit&7))) & 1;
        } else bit = 1;
        value |= (!bit) << p;
    }
    samples++; event('R',value); return value;
}
static void io_write(device_t *dev, unsigned address, unsigned char value) {
    (void)dev;
    if (address == 2) { event('C',value); return; }
    assert(address == 0);
    event('W',value);
    if (!(pins & 4) && (value & 4)) { index_bit=0; latches++; }
    if (!(pins & 8) && (value & 8)) {
        if (value & 4) {
            cycles++;
            if (!clone) for (unsigned p=0;p<2;p++) speeds[p]=(speeds[p]+1)%3;
        } else index_bit++;
    }
    pins=value;
}
static unsigned symbol(const char *map, const char *name) {
    FILE *f=fopen(map,"r"); assert(f);
    char line[512], s[128]; unsigned addr;
    while (fgets(line,sizeof(line),f)) {
        if (sscanf(line," %x %127s",&addr,s)==2 && s[0]=='_' &&
            (strlen(s)==32 ? !strncmp(name,s,32) : !strcmp(name,s))) { fclose(f); return addr; }
    }
    fprintf(stderr,"Missing symbol %s in %s\n",name,map); abort();
}
static void load(const char *path) {
    FILE *f=fopen(path,"r"); assert(f);
    char line[1024]; memset(image,0,sizeof(image));
    while (fgets(line,sizeof(line),f)) {
        unsigned n,addr,type;
        assert(sscanf(line,":%2x%4x%2x",&n,&addr,&type)==3);
        if (type != 0) continue;
        for(unsigned i=0;i<n;i++) {
            unsigned b; assert(sscanf(line+9+2*i,"%2x",&b)==1);
            assert(addr+i<65536); image[addr+i]=b;
        }
    }
    fclose(f);
}
static void setup(void) {
    vtimer_init(); z80_init(&cpu); memcpy(memory,image,sizeof(memory));
    device_init_mem(&ram,"test RAM",mem_read,mem_write,sizeof(memory));
    mmu_register_mem_device(&cpu.mmu,0,&ram);
    for(unsigned i=0;i<4;i++) { cpu.mmu.pages[i]=i; cpu.mmu.vpages[i].dev=&ram; }
    device_init_io(&pio,"SNES PIO",io_read,io_write,4);
    mmu_register_io_device(&cpu.mmu,0xd0,&pio);
    pins=8; index_bit=cycles=latches=samples=steps=nevents=0;
    headers[0]=headers[1]=0; memset(axes,0,sizeof(axes));
    speeds[0]=speeds[1]=0; sensitivity=clone=0;
}
static unsigned long call(unsigned addr, unsigned a, unsigned hl, unsigned de) {
    cpu.pc=addr; cpu.a=a; cpu.h=hl>>8; cpu.l=hl; cpu.d=de>>8; cpu.e=de;
    cpu.sp=0xff00; memory[cpu.sp]=0; memory[cpu.sp+1]=1;
    cpu.ix=0x1234; cpu.iy=0x5678;
    cpu.a_=0x12;cpu.b_=0x34;cpu.c_=0x56;cpu.d_=0x78;cpu.e_=0x9a;cpu.h_=0xbc;cpu.l_=0xde;cpu.f_=0xf0;
    int iff1=cpu.iff1,iff2=cpu.iff2;
    unsigned long before=cpu.cyc;
    while(cpu.pc != 0x100) {
        assert(cpu.cyc-before<1000000); z80_step(&cpu); steps++;
    }
    assert(cpu.sp==0xff02 && cpu.ix==0x1234);
    if(current_image) {
        assert(cpu.iy==0x5678 && cpu.iff1==iff1 && cpu.iff2==iff2);
        assert(cpu.a_==0x12 && cpu.b_==0x34 && cpu.c_==0x56 && cpu.d_==0x78 && cpu.e_==0x9a && cpu.h_==0xbc && cpu.l_==0xde && cpu.f_==0xf0);
    }
    return cpu.cyc-before;
}
static unsigned word(unsigned address) { return memory[address]|memory[address+1]<<8; }

struct timing {
    unsigned long latch, first, header_low, header_first_high, header_high, header_settle;
    unsigned long mouse_low, mouse_high, mouse_settle, initial_gap, byte_gap, continuation_gap;
    unsigned long cycle_before, cycle_low, cycle_after, report_recovery, cycle_recovery;
};
static struct timing minima(void) {
    struct timing m; memset(&m,0xff,sizeof(m));
    unsigned previous=8; unsigned long last_latch=0,last_rise=0,last_fall=0,last_sample=0,cycle_edge=0,cycle_done=0;
    int first=0,cycle_stage=0;
    for(unsigned i=0;i<nevents;i++) {
        struct event e=events[i];
        if(e.kind=='W') {
            if(!(previous&4) && (e.value&4)) {
                if(last_sample && !cycle_done) { unsigned long v=e.t-last_sample; if(v<m.report_recovery)m.report_recovery=v; }
                if(cycle_done) { unsigned long v=e.t-cycle_done; if(v<m.cycle_recovery)m.cycle_recovery=v; cycle_done=0; }
                last_latch=e.t; cycle_edge=e.t; first=1;
            }
            if(previous==12 && e.value==4) {
                unsigned long v=e.t-cycle_edge; if(v<m.cycle_before)m.cycle_before=v;
                cycle_edge=e.t; cycle_stage=1;
            }
            if(previous==4 && e.value==12) {
                unsigned long v=e.t-cycle_edge; if(v<m.cycle_low)m.cycle_low=v;
                cycle_edge=e.t; cycle_stage=2;
            }
            if((previous&4) && !(e.value&4)) {
                unsigned long v=e.t-last_latch;
                if(cycle_stage==2) { v=e.t-cycle_edge; if(v<m.cycle_after)m.cycle_after=v; cycle_done=e.t; }
                else if(v<m.latch)m.latch=v;
                cycle_stage=0; last_rise=e.t;
            }
            if(previous==8 && e.value==0) {
                last_fall=e.t;
                unsigned long v=e.t-last_rise;
                if(e.index==0) { if(v<m.header_first_high)m.header_first_high=v; }
                else if(e.index<15) { if(v<m.header_high)m.header_high=v; }
                else if(e.index==15) { v=e.t-last_sample; if(v<m.initial_gap)m.initial_gap=v; }
                else if(e.index==31) { v=e.t-last_sample; if(v<m.continuation_gap)m.continuation_gap=v; }
                else if((e.index&7)==7) { v=e.t-last_sample; if(v<m.byte_gap)m.byte_gap=v; }
                else if(v<m.mouse_high)m.mouse_high=v;
            }
            if(previous==0 && e.value==8) {
                unsigned long v=e.t-last_fall;
                if(e.index<15) { if(v<m.header_low)m.header_low=v; }
                else if(v<m.mouse_low)m.mouse_low=v;
                last_rise=e.t;
            }
            previous=e.value;
        } else if(e.kind=='R') {
            unsigned long v=e.t-last_rise;
            if(e.index==0 && first) { if(v<m.first)m.first=v; }
            else if(e.index<16) { if(v<m.header_settle)m.header_settle=v; }
            else if(v<m.mouse_settle)m.mouse_settle=v;
            last_sample=e.t;
        }
    }
    return m;
}
static void merge(struct timing *to,struct timing from) {
    unsigned long a[sizeof(from)/sizeof(unsigned long)],b[sizeof(from)/sizeof(unsigned long)];
    memcpy(a,to,sizeof(from));memcpy(b,&from,sizeof(from));
    for(unsigned i=0;i<sizeof(from)/sizeof(unsigned long);i++) if(b[i]<a[i])a[i]=b[i];
    memcpy(to,a,sizeof(from));
}
static void compare(const char *label,struct timing got,struct timing need) {
    static const char *names[]={"latch","first sample","header low","first clock high","header high","header settling",
        "mouse low","mouse high","mouse settling","initial mouse gap","mouse byte gap","detection continuation gap",
        "cycle setup","cycle low","cycle finish","report recovery","cycle recovery"};
    unsigned long a[sizeof(got)/sizeof(unsigned long)],b[sizeof(need)/sizeof(unsigned long)];
    memcpy(a,&got,sizeof(got));memcpy(b,&need,sizeof(need));
    for(unsigned i=0;i<sizeof(got)/sizeof(unsigned long);i++) if(b[i]!=ULONG_MAX) {
        if(a[i]==ULONG_MAX || a[i]<b[i]) {
            fprintf(stderr,"%s %s: got %lu T; baseline requires %lu T\n",label,names[i],a[i],b[i]);
            abort();
        }
    }
}
static void print_timing(const char *label, struct timing m) {
    static const char *names[]={"latch","first","header-low","first-high","header-high","header-settle",
        "mouse-low","mouse-high","mouse-settle","start-gap","byte-gap","continuation-gap",
        "cycle-setup","cycle-low","cycle-finish","report-recovery","cycle-recovery"};
    unsigned long values[sizeof(m)/sizeof(unsigned long)];
    memcpy(values,&m,sizeof(m));
    printf("%s minima (T):",label);
    for(unsigned i=0;i<sizeof(values)/sizeof(values[0]);i++)
        if(values[i]!=ULONG_MAX)printf(" %s=%lu",names[i],values[i]);
    puts("");
}
/* Independent protocol bounds at 10 MHz, not old C instruction counts.
 * Jim Christy: 12 us latch, 6 us phases/settling.
 * NESdev Mouse: 170/336 SNES master clocks => round up to 8/16 us.
 */
static void check_pad_timing(void) {
    struct timing need; memset(&need,0xff,sizeof(need));
    need.latch=120; need.first=60; need.header_low=60;
    need.header_first_high=60; need.header_high=60; need.header_settle=60;
    struct timing got=minima(); compare("pad protocol",got,need);
    /* Guard against accidentally reinstating the old mouse-only padding. */
    assert(got.latch<=140 && got.first<=120 && got.header_low<=80);
    assert(got.header_first_high<=200 && got.header_high<=200 && got.header_settle<=80);
}
static void check_hyperkin_timing(void) {
    unsigned long previous=0; unsigned previous_bit=0;
    for(unsigned i=0;i<nevents;i++) if(events[i].kind=='R') {
        if(previous) {
            assert(events[i].index==previous_bit+1);
            unsigned long required=events[i].index==16 ? 160 : 80;
            if(events[i].t-previous<required) {
                fprintf(stderr,"Hyperkin bit %u: %lu T between reads, need %lu T\n",
                    events[i].index,events[i].t-previous,required);
                abort();
            }
        }
        previous=events[i].t;previous_bit=events[i].index;
    }
}
static int axis(unsigned raw) { int mag=raw&127; return raw&128 ? -mag:mag; }
static void unchanged_interrupts(unsigned iff) {
    assert(cpu.iy==0x5678 && cpu.iff1==!!iff && cpu.iff2==!!iff);
}
static void check_current(const char *map, struct timing pad[2],struct timing movement[2],
                          struct timing speed[2],struct timing detect[2],struct timing cycle[2]) {
    unsigned one=symbol(map,"_controller_read_pad"),two=symbol(map,"_controller_read_pads");
    unsigned mouse=symbol(map,"_controller_read_mouse"),ispad=symbol(map,"_controller_is_pad");
    unsigned ismouse=symbol(map,"_controller_is_mouse"),get=symbol(map,"_controller_get_mouse_sensitivity");
    unsigned set=symbol(map,"_controller_set_mouse_sensitivity");
    unsigned init=symbol(map,"_controller_init");
    unsigned long tpad=0,tmouse=0; unsigned ipad=0,imouse=0;
    for(unsigned iff=0;iff<2;iff++) {
        setup();cpu.iff1=cpu.iff2=iff;
        call(init,0,0,0); assert(cpu.a==0 && nevents==4 && pins==8);
        assert(events[0].kind=='C' && events[0].value==0xcf);
        assert(events[1].value==3 && events[2].value==3);
        unchanged_interrupts(iff);
    }
    for(unsigned p=0;p<2;p++) {
        for(unsigned v=0;v<65536;v+=17) {
            setup();headers[p]=v;headers[1-p]=v^0x5aa5;cpu.iff1=cpu.iff2=v&1;
            tpad=call(one,p,0,0);ipad=steps;
            if(p==0 && v==0)print_timing("ASM pad",minima());
            assert((cpu.d<<8|cpu.e)==v && samples==16 && latches==1 && index_bit==15 && pins==8);
            check_pad_timing(); unchanged_interrupts(v&1);
        }
    }
    for(unsigned v=0;v<65536;v+=257) {
        setup();headers[0]=v;headers[1]=v^0x96a5;
        call(two,0,0x9000,0);
        if(word(0x9000)!=v || word(0x9002)!=(v^0x96a5)) { fprintf(stderr,"dual: v=%04x got %04x/%04x expected %04x/%04x\n",v,word(0x9000),word(0x9002),v,v^0x96a5); abort(); }
        assert(samples==16 && latches==1 && index_bit==15);
        check_pad_timing();unchanged_interrupts(0);
    }
    for(unsigned bit=0;bit<16;bit++) {
        setup();headers[0]=1u<<bit;headers[1]=1u<<((bit+7)&15);
        call(two,0,0x9000,0);
        assert(word(0x9000)==headers[0] && word(0x9002)==headers[1]);
    }
    for(unsigned p=0;p<2;p++) for(unsigned raw=0;raw<256;raw++) {
        setup();headers[p]=0x8300; axes[p][0]=raw;axes[p][1]=raw^0x55;
        cpu.iff1=cpu.iff2=raw&1;
        tmouse=call(mouse,p,0,0x9000);imouse=steps;
        if(p==0 && raw==0)print_timing("ASM mouse",minima());
        assert(word(0x9000)==0x8300);
        assert((signed char)memory[0x9002]==axis(raw^0x55));
        assert((signed char)memory[0x9003]==axis(raw));
        assert(cpu.a==((axis(raw)!=0?1:0)|(axis(raw^0x55)!=0?2:0)));
        assert(samples==32 && index_bit==31 && latches==1 && pins==8);
        struct timing want=pad[p];
        want.mouse_low=movement[p].mouse_low;want.mouse_high=movement[p].mouse_high;
        want.mouse_settle=movement[p].mouse_settle;want.byte_gap=movement[p].byte_gap;
        /* Includes old header function's return path before movement entry. */
        want.initial_gap=movement[p].initial_gap;
        compare("mouse",minima(),want); check_hyperkin_timing(); unchanged_interrupts(raw&1);
    }
    for(unsigned p=0;p<2;p++) for(unsigned h=0;h<16;h++) {
        setup(); headers[p]=(h<<12)|0x300;
        call(ispad,p,0,0);assert(cpu.a==(h==0));assert(samples==16);check_pad_timing();
        setup(); headers[p]=(h<<12)|0x300; memset(memory+0x9000,0xff,4);
        call(mouse,p,0,0x9000);
        assert(cpu.a==(h==8?0:255)); if(h!=8)assert(word(0x9000)==0 && word(0x9002)==0);
        setup();headers[p]=(h<<12)|0x300;
        call(ismouse,p,0,0);assert(cpu.a==(h==8));assert(samples==48 && index_bit==47);
        compare("mouse detection",minima(),detect[p]); check_hyperkin_timing();
    }
    /* Calls emitted by SDCC verify actual register arguments and returns. */
    setup();headers[0]=0x1234;call(symbol(map,"_test_pad1"),0,0,0);assert((cpu.d<<8|cpu.e)==0x1234);
    setup();headers[1]=0xabcd;call(symbol(map,"_test_pad2"),0,0,0);assert((cpu.d<<8|cpu.e)==0xabcd);
    setup();headers[0]=0x1234;headers[1]=0xabcd;call(symbol(map,"_test_pads"),0,0,0);assert(word(0x9000)==0x1234 && word(0x9002)==0xabcd);
    setup();headers[0]=0x8000;axes[0][0]=0x81;axes[0][1]=0x7f;call(symbol(map,"_test_mouse1"),0,0,0);assert(cpu.a==3 && memory[0x9002]==127 && memory[0x9003]==255);
    setup();headers[1]=0x8000;axes[1][1]=1;call(symbol(map,"_test_mouse2"),0,0,0);assert(cpu.a==2 && memory[0x9002]==1);
    setup();sensitivity=1;headers[0]=0x8000;call(symbol(map,"_test_set1"),0,0,0);assert(cpu.a==2 && speeds[0]==2);
    setup();sensitivity=1;headers[1]=0x8000;call(symbol(map,"_test_set2"),0,0,0);assert(cpu.a==1 && speeds[1]==1);
    const unsigned edge[]={0,128,1,129,127,255};
    for(unsigned x=0;x<6;x++) for(unsigned y=0;y<6;y++) {
        setup();headers[0]=0x8000;axes[0][0]=edge[y];axes[0][1]=edge[x];
        call(mouse,0,0,0x9000);
        assert((signed char)memory[0x9002]==axis(edge[x]) && (signed char)memory[0x9003]==axis(edge[y]));
        assert(cpu.a==((axis(edge[x])?2:0)|(axis(edge[y])?1:0)));
    }
    for(unsigned p=2;p<256;p++) {
        setup();call(one,p,0,0);assert((cpu.d<<8|cpu.e)==0xffff && nevents==0);
        call(mouse,p,0,0x9000);assert(cpu.a==255 && nevents==0);
        call(ispad,p,0,0);assert(cpu.a==0 && nevents==0);
        call(ismouse,p,0,0);assert(cpu.a==0 && nevents==0);
        call(get,p,0,0);assert(cpu.a==255 && nevents==0);
        call(set,p,1,0);assert(cpu.a==255 && nevents==0);
    }
    setup();call(two,0,0,0);assert(nevents==0);
    call(mouse,0,0,0);assert(cpu.a==255 && nevents==0);
    for(unsigned p=0;p<2;p++)for(unsigned level=0;level<3;level++) {
        setup();sensitivity=1;headers[p]=0x8000;speeds[p]=level;
        call(get,p,0,0);assert(cpu.a==level);compare("sensitivity",minima(),speed[p]);check_hyperkin_timing();
        for(unsigned target=0;target<3;target++) {
            setup();sensitivity=1;headers[p]=0x8000;speeds[p]=level;
            call(set,p,target,0);assert(cpu.a==target && speeds[p]==target);
            assert(cycles>=1 && cycles<=3);compare("sensitivity cycles",minima(),cycle[p]);
            unsigned oldcycles=cycles;
            call(set,p,target,0);assert(cpu.a==target && cycles==oldcycles);
        }
    }
    setup();sensitivity=1;headers[0]=headers[1]=0x8000;
    call(set,0,0,0);assert(cpu.a==0 && cycles==3);
    call(set,1,0,0);assert(cpu.a==0 && cycles==6); /* independent first-use state */
    for(unsigned p=0;p<2;p++)for(unsigned target=0;target<3;target++) {
        setup();sensitivity=clone=1;headers[p]=0x8000;
        call(set,p,target,0);assert(cpu.a==(target==0?0:255) && cycles==(target==0?1:3));
    }
    for(unsigned v=3;v<256;v++) { setup();call(set,0,v,0);assert(cpu.a==255 && nevents==0); }
    setup(); headers[0]=0;call(get,0,0,0);assert(cpu.a==255);
    setup(); headers[0]=0x8c00;call(get,0,0,0);assert(cpu.a==255);
    printf("ASM: single pad %lu T / %u instructions; mouse %lu T / %u instructions\n",tpad,ipad,tmouse,imouse);
    puts("controller: reports, decoding, timing minima, detection, sensitivity, invalid inputs and ABI passed");
}
int main(int argc,char **argv) {
    assert(argc==5); load(argv[1]);
    struct timing pad[2],movement[2],speed[2],detect[2],cycle[2];
    memset(&pad,0xff,sizeof(pad));memset(&movement,0xff,sizeof(movement));
    memset(&speed,0xff,sizeof(speed));memset(&detect,0xff,sizeof(detect));memset(&cycle,0xff,sizeof(cycle));
    unsigned long baseline_pad=0,baseline_mouse=0;unsigned ip=0,im=0;
    for(unsigned p=0;p<2;p++)for(unsigned v=0;v<4;v++) {
        setup();headers[0]=v&1?0xffff:0;headers[1]=v&2?0xffff:0;
        baseline_pad=call(symbol(argv[2],"_controller_read_port"),p,0,0);ip=steps;
        struct timing t=minima();
        /* Reference samples both lines. Select the requested line's samples. */
        unsigned write=0,reads=0;
        for(unsigned i=0;i<nevents;i++) {
            if(events[i].kind=='R' && (reads++%2)!=p)continue;
            events[write++]=events[i];
        }
        nevents=write;t=minima();merge(&pad[p],t);
        /* Continue immediately, including original header return/decode overhead. */
        unsigned long last=events[nevents-1].t; nevents=steps=samples=0;
        baseline_mouse=call(symbol(argv[2],"_controller_read_mouse"),p,0,0);im=steps;
        t=minima(); t.initial_gap=events[0].t-last; merge(&movement[p],t);
        setup();headers[p]=0x8000;sensitivity=1;
        call(symbol(argv[2],"_controller_get_mouse_sensitivity"),p,0,0);merge(&speed[p],minima());
        setup();headers[p]=0x8000;sensitivity=1;
        call(symbol(argv[2],"_controller_set_mouse_sensitivity"),p,1,0);merge(&cycle[p],minima());
        setup();headers[p]=0x8000;
        call(symbol(argv[2],"_controller_is"),p,1,0);merge(&detect[p],minima());
    }
    setup(); headers[0]=headers[1]=0xffff;
    baseline_pad=call(symbol(argv[2],"_controller_read_port"),1,0,0);ip=steps;
    setup();headers[1]=0x8300;axes[1][0]=255;axes[1][1]=170;
    baseline_mouse=call(symbol(argv[2],"_controller_read_port"),1,0,0);
    baseline_mouse+=call(symbol(argv[2],"_controller_read_mouse"),1,0,0);im=steps;
    printf("C benchmark: pad %lu T / %u instructions; full mouse %lu T / %u instructions\n",baseline_pad,ip,baseline_mouse,im);
    print_timing("pad port 1",pad[0]);print_timing("pad port 2",pad[1]);print_timing("movement",movement[0]);print_timing("sensitivity",speed[0]);
    load(argv[3]);current_image=1;check_current(argv[4],pad,movement,speed,detect,cycle);
    return 0;
}

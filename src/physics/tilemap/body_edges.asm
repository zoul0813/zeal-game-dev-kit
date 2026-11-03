    .module physics_body_edges
    .globl _body_edges
    .globl _body_edge_left, _body_edge_right, _body_edge_top, _body_edge_bottom
    .area _TEXT

; HL = body; fills shared pixel edges from signed 12.4 positions and dimensions.
; Clobbers AF/BC/DE/HL; IX/IY preserved.
_body_edges::
    ; hl = body
    ld      e, (hl)
    inc     hl
    ld      d, (hl)
    inc     hl
    sra     d
    rr      e
    sra     d
    rr      e
    sra     d
    rr      e
    sra     d
    rr      e
    ld      (_body_edge_left), de
    ld      c, (hl)
    inc     hl
    ld      b, (hl)
    sra     b
    rr      c
    sra     b
    rr      c
    sra     b
    rr      c
    sra     b
    rr      c
    ld      (_body_edge_top), bc
    push    bc
    ld      bc, #5
    add     hl, bc
    ld      c, (hl)
    ld      b, #0
    dec     bc
    ex      de, hl
    add     hl, bc
    ld      (_body_edge_right), hl
    ex      de, hl
    inc     hl
    ld      c, (hl)
    ld      b, #0
    dec     bc
    pop     hl
    add     hl, bc
    ld      (_body_edge_bottom), hl
    ret

; Scratch belongs to the edge helper so linking it does not pull in body_step().
; Every call writes all four values. The collision step is not reentrant.
    .area _DATA
_body_edge_left::
    .ds 2
_body_edge_right::
    .ds 2
_body_edge_top::
    .ds 2
_body_edge_bottom::
    .ds 2

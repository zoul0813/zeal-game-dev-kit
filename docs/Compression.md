# ZGDK: Compression

ZGDK has built-in RLE decompression available via `decompress(COMPRESSION_RLE, &data, size, &buffer)`.

This will decompress data using the same RLE algorithm used by
[Zeal Video Board](https://github.com/zeal8bit/Zeal-VideoBoard-SDK) for sprites.

This can be useful if you'd like to RLE compress Tilemap data, Palette data, or any other data that contains
repeated chunks and benefits from the RLE compression algorithm.

For external or untrusted assets, use
`decompress_rle_checked(data, compressed_size, buffer, capacity)`. It accepts
16-bit sizes and returns `false` for empty or malformed streams or insufficient
output capacity, without modifying the destination. The existing `decompress_rle`
and `decompress` APIs ignore malformed streams but still require enough output
storage from the caller. Input and output buffers must not overlap.

# ZGDK: Compression

ZGDK has built-in RLE decompression available via `decompress(COMPRESSION_RLE, &data, size, &buffer)`.

This will decompress data using the same RLE algorithm used by
[Zeal Video Board](https://github.com/zeal8bit/Zeal-VideoBoard-SDK) for sprites.

This can be useful if you'd like to RLE compress Tilemap data, Palette data, or any other data that contains
repeated chunks and benefits from the RLE compression algorithm.

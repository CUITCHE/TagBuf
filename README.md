# TagBuf
TagBuf is a network data exchange format.

# Key Words
- tagBuf: tag + data
- tagBuffer: N tagBufs
# What is Tag
Tag contains much information about data. TagBuf writes or reads from blob data to build a tagBuffer by tagBuf and build a tagBuf by tag.
# What's the Tag's Structure
Tag takes up 4 bytes. And tag will make the best of each bit.
|Range of Bits | Note |
|---|---|
|31~13|field number|
|12~08|tagBuf reserved|
|07~04|tagBuf internal-tag|
|03~00|tagBuf write type|
## TagBuf Write Type (00~03)
|No.|Type|Note|
|---|---|---|
|0|Varint|kinds of [singed\|unsigned]int(8,16,31,64bits)|
|1|float|May be deprecated in the future|
|2|double|May be deprecated in the future|
|3|container|such as array(list)|
|4|blob stream|such as string, blob data|
## TagBuf Internal-Tag (04~07)
- 04: 1 represents integer was compressed by zigzag if write-type is about integer or string's length. And the placeholder is valid only write-type is about integer or string. 0 represents not.
- 05: 1 represents there is a data of container like array at next. 0 represents not.
- 06: 1 represents there is a tagBuf cell at next. 0 represents not.
- 07: 1 represents there is a tagBuf object at next. 0 represents not.
## TagBuf Reserved (08~12)
May make reserved for shared space.

Continue...
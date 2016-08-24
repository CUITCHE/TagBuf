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
|No.|Write Type|Note|
|---|---|---|
|0|VarintFixed|kinds of Integers, zigzag-int, zigzag-int64. Above also support unsinged type. double value, 8 bytes, float value, 4 bytes. And about bool value, specially it only takes up 1 bit.|
|1|Container|NSArray on iOS platform(Objective-C), List on Android(Java), vector\<T> on C++|
|2|Blob Stream|such as NSString, NSData/String/string|
|3|Object|An Object|

## TagBuf Internal-Tag (04~07)
Especially, if [07] eaquals to 0 represents there is an object at next tagBuf, 1 represents not.

- Write type is VarintFixed.

|Internal-Tag value|Meaning|
|---|---|
|0|8 bits integer, such as char|
|1|16 bits integer, such as short|
|2|32 bits integer, such as int|
|3|64 bits integer, such as long/long long|
|4|32 bits float|
|5|64 bits double|
|6|1 bits value: bool|
|7|Indicate object is nil/null if this is a number(NSNumber/Number) object|

- Write type is Container

|Internal-Tag value|Meaning|
|---|---|
|0|Indicate non-contain, such as a array contain 0 element.|
|1|Contains string|
|2|Contains data(NSData)|
|3|Contains an object|
|4|Contains containers|
|7|Indicate object is nil/null/NULL|

You may have a question: where is integer. We think it is an object in container's integer(include C++). We will check type of container's element and store it to builtin structure in runtime. Defaultly, contain-type is 32 bits intger if the type is like integer(include float and double).

- Write type is Blob Stream

|Internal-Tag value|Meaning|
|---|---|
|0|Represents NSString/String/string|
|1|Represents NSData|
|7|Indicate object is nil/null/NULL|

- Write type is Blob Stream

|Internal-Tag value|Meaning|
|---|---|
|7|Indicate object is nil/null/NULL|

## TagBuf Reserved (08~12)
|Bits|Meaning|
|---|---|
|08~10|Represents the length of zigzag|
|11|1 represents that the bytes of string or data's length has been compressed by zigzag.|
|12|1 represents that there store a bool [true] value, 0 represents [false] value when write tpye is VarintFixed and internal-tag is varint_bool(6). And 1 represents that container contains number value(bool, 8~64bits, values by VarintFixed's internal values).|

Above is overall design idea.

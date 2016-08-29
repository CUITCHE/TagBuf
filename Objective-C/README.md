# TagBuf - Implementation In Objective-C
TagBuf is implemented by C++ and Objective-C. And use inline template to serialize data to improve executed speed.

# Performance - Compared To JSONModel
We make a Class - `CHPerformanceTestModel` and fill some data to its objects.

```Objective-C
- (void)fillTestData
{
    self.name = 0x23546;
    self.prority = 5346.65473;
    self.number = 35;
    self.opened = true;
    NSMutableString *str = [NSMutableString stringWithCapacity:100];
    int i=100;
    while (i-->0) {
        [str appendString:@"1"];
    }
    self.str100Length = str;

    self._10StringContent = @[str,str,str,str,str,str,str,str,str,str];
}
```
- It is 1220 bytes data by TagBuf generated.
- It is 1231 bytes data by JSONModel generated.

## Performance Show
We test the generated buf/string and synthetic object 1 million times. Below is data table.

||Generated(avg)|Synthetic(avg)|
|---|---|---|
|TagBuf|5.27sec|6.35sec|
|JSONModel|18.25sec|20.80sec|
<iframe style="width: 100%; height: 100%" src="http://www.tubiaoxiu.com/p.html?s=71faf114154fe56b"></iframe>

We test 10 times that test 100 thousands every times.
||0|1|2|3|4|5|6|7|8|9|
|---|---|---|---|---|---|---|---|---|---|---|
|TagBuf generated|0.538884|0.504202|0.512431|0.583138|0.520764|0.519910|0.517318|0.520601|0.530910|0.523206|
|TagBuf synthetic|0.639692|0.627528|0.623658|0.629398|0.639663|0.642257|0.636524|0.632508|0.643452|0.634625|
|JSONModel generated|1.861764|1.827200|1.730534|1.714605|1.735006|2.000661|1.872454|2.081739|1.725383|1.700310|
|JSONModel synthetic|2.162973|2.038030|2.046029|2.047985|2.076460|2.021429|2.044203|2.038306|2.050623|2.269900|
<iframe style="width: 100%; height: 100%" src="http://www.tubiaoxiu.com/p.html?s=bdbf541e344572c9"></iframe>
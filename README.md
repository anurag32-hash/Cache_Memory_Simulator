## Cache Memory Simulator

In this I have developed a simulation software for cache memory. The simulated
cache will have the relevant functionality, but with a _new Replacement Policy_ that works as follows. Each Cache Set is divided into two groups:

1. One group contains the HIGH PRIORITY lines of the set
2. The other group contains the LOW PRIORITY lines of the set

How is _priority_ established? If a line is accessed again after the initial access that fetches it into
the cache, it is promoted to the HIGH PRIORITY group. If a line is not accessed for sufficiently
long (T cache accesses) after being moved to the HIGH PRIORITY group, it is moved to the
LOW PRIORITY group. Within a priority group, the Least Recently Used policy may be used to
manage the lines.

**Inputs** to the software will be: (inputs to be taken from input file)

1. Cache size
2. Cache line/block size
3. Cache associativity
4. The value of T
5. A sequence of memory access requests: Memory address, R or W (for Read or Write),
    and Data (if Write)

**Outputs** from the software will be: (output the content and cache statistics to output file)

1. The complete content of the cache (Data, Tag, and Control bits of every cache line) in
each Way after the memory access sequence is complete.
2. Cache statistics:
a. Number of Accesses
b. Number of Reads, Read Hits, and Read Misses
c. Number of Writes, Write Hits, and Write Misses
d. Hit Ratio


## Assumptions

1. I have considered memory addresses as byte addressable because I could
    clearly see the spatial locality and furthermore, I’ve considered 1 int = 1
    byte. This implies if a block size is n bytes then it can store n integers.
2. According to the assumption 1 we should input data only from 0−255,
    but just to show changes in the cache and in the main memory more
    profoundly, we can input any int. Although I also have incorporated the
    functionality according to the assumption 1.
3. In case of Write, if a new block is added in the cache, dirty bit is modified
    to 1. In case of Write Miss, we generate a new block from Main Memory
    and insert at the appropriate place in the cache enhancing the spatial
    locality.
4. In case of Read, the appropriate cache block is returned. In case of Read
    Miss, we generate a new block from Main Memory and insert at the ap-
    propriate place in the cache enhancing the spatial locality.
5. While finding a replacement candidate in cache, LRU is first applied in
    LOW Priority Blocks and if there are no Low Priority Blocks then LRU
    is applied in the whole set. And in LRU we select the block which has the
    lowest Access Number.

## Implementation

1. startCache- Used for initializing the cache and allied vectors.
2. cached- To lookup in the cache and on a Miss, calling out appropriate
    functions.
3. readHM- To decide whether it’s a HIT or Miss.
4. getNewBlock- If it is a MISS, to generate new block from Main memory.
5. insert- To insert the new block in the cache at the appropriate place.
6. replacement- It returns the index of the block which is to be replaced by
    applying LRU on LOW Priority Blocks.
7. eviction- It writes the contents of the block to be replaced in the Main
    Memory.

Note- High and Low priority groups are not fixed in size. There sizes can
vary at runtime. This is done to take the the full advantage of the cache asso-
ciativity. Suppose if we fix the sizes of both the groups, and we need a place in
High Priority Group, then we had to evict a high priority block whereas in this
dynamic settings we can directly make that block itself a part of High Priority
Group.



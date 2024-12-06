

# Containers

|Container|Cpp Type|API Ident|Notes|
|---|---|---|---|
|Array|std::vector|ARR_|Fixed size|
|List|std::list/std::forward_list|LST_|Use forward_list unless we need bidirection|
|Queue|std::queue|QUE_|Use std::vector as underlying container|
|Set|ankerl|SET_|set or segmented_set, also check `std::unordered_set`|


## Issues
- How to persist



## Containers

- Note: most commands require a `name`, even if not shown

### Array

A fixed size vector.

- Offers special commands compared to creating a key with array value (set/get items by position, swapping, union, etc)
- Offers spacial locality benefits compared to List. May offer benefits over a Set, depending on commands available.

|Command|Purpose|Args|Notes|
|---|---|---|---|
|ARR_CREATE|Creates array|`name`,`len`|Call `reserve()` with a clamped `len`|
|ARR_DELETE|Deletes entire list|||
|ARR_LEN|Returns `size()`|||
|ARR_GET|Returns a single item|`pos`||
|ARR_GET_RNG|Returns multiple, based on range|`rng:[a,b]`|Decide if `[a,b]` or `[a,b)`|
|ARR_SET|Overwrites existing item|`pos`,`item`||
|ARR_SWAP_ITEM|Swaps two items|`pos1`, `pos2`||


### List
Suitable for inserts at random positions at the expensive of spacial locality.

TODO
- Think about intersections/unions

|Command|Purpose|Args|Notes|
|---|---|---|---|
|LST_CREATE|Creates list|`name`,`lenHint`|Call `reserve()` with a clamped `lenHint`|
|LST_DELETE|Deletes entire list|||
|LST_LEN|Returns `size()`|||
|LST_COPY|Creates a copy of this list|`src_pos`, `dest_pos`, `len`, `destination`|Creates a new list `destination`, copying nodes from [`src_pos`,`src_pos+len`] to `dest_pos`|
|LST_MOVE|Move nodes to a different list|`src_pos`, `dest_pos`, `len`, `destination`|Moves nodes from [`src_pos`, `src_pos+len`] to `destination` at `dest_pos`. Create `destination` if it does not exist. See `std::list::splice()`|
|LST_SET_HEAD|Add to head of list|`item`|Returns `size()`|
|LST_SET_TAIL|Add to tail of list|`item`|Returns `size()`|
|LST_SET|Add by position|`item`, `pos`|If `pos > size()`, add to tail. If `pos < 0 or empty()` add to head. Returns `pos`|
|LST_GET_HEAD|Get head item|||
|LST_GET_TAIL|Get tail item|||
|LST_GET|Get an item by position|`pos`|Decide what to return if `pos` out of bounds|
|LST_RMV_HEAD|Delete head||Return `size()`|
|LST_RMV_TAIL|Delete head||Return `size()`|
|LST_RMV|Delete by position|`pos`, `len`|Removes from `pos` to `pos+len`. `pos` must be in range, but if `pos+len > size()` remove from `pos` to end. Returns `size()`|


### Queue
Standard queue backed by `std::vector`.

|Command|Purpose|Args|Notes|
|---|---|---|---|
|QUE_CREATE|Create queue|`name`, `lenHint`|Call `reserve()` on backing vector with a clamped `lenHint`|
|QUE_DELETE|Delete queue|`name`||
|QUE_COPY|Creates a new queue, as a copy|`src_name`, `dest_name`|Create `dst_name` queue, copying from `src_name`|
|QUE_PUSH|Push one item|`item`||
|QUE_PUSH_M|Push multiple item|`items`||
|QUE_POP|Pop|||
|QUE_POP_M|Pop multiple|`qty`|Pops `qty` elements from queue|
|QUE_FIRST|Returns first item, does not pop|||
|QUE_LAST|Returns last item, does not pop|||


### Set
|Command|Purpose|Args|Notes|
|---|---|---|---|
|SET_CREATE|Create|`name`||
|SET_DELETE|Delete|`name`||
|SET_ADD|Add|`name`||


# Containers

|Container|Cpp Type|API Ident|Notes|
|---|---|---|---|
|Array|std::vector|ARR_|Fixed size arrays, of various data types: object, integer, string (possibly byte)|
|List|std::list/std::forward_list|LST_|Use forward_list unless we need bidirection|
|Queue|std::queue|QUE_|Use std::vector as underlying container|
|Set|ankerl|SET_|set or segmented_set, also check `std::unordered_set`|


# Issues
- How to persist



# Array

A fixed size vector.

- Offers special commands compared to creating a key with array value (set/get items by position, swapping, union, etc)
- Offers spacial locality benefits compared to List. May offer benefits over a Set, depending on commands available.

<br/>

`OARR_` : object array
`IARR_` : integer array (int or unsigned)

> NOTE
>
> `IARR` stores values as `std::int64_t` but allows unsigned values, this limits max  value.
>

## Initial

Note: all commands require a `name`, even if not shown, except `ARR_DELETE_ALL`

|Command|Purpose|Args|Notes|
|---|---|---|---|
|_CREATE|Creates array|`name`,`len`|Call `resize()`. Items are empty json object|
|_DELETE|Deletes array|||
|_DELETE_ALL|Deletes all arrays||Useful for testing, maybe not production|
|_EXISTS|Checks array name|`name`||
|_LEN|Returns `size()`|||
|_GET|Returns a single item|`pos`||
|_GET_RNG|Returns multiple, based on range|`rng:[a,b]` or `rng:[a]`|Range is: `[a,b)`. If `b` not set or past end, assume `end()`. Allow negative indices?|
|_SET|Overwrites existing item|`pos`,`item`||
|_SET_RANGE|Overwrites existing items|`pos`,`items`|Sets items array, starting at `pos`|
|_SWAP|Swaps two items|`posA`, `posB`||
|_CLEAR|Resets items to empty objects|`name`, `rng:[a,b]`|Sets elements in `[a,b)` to empty object|


---
<br/>

### IArray
|Command|Purpose|Args|Notes|
|---|---|---|---|
|_INTERSECT|Intersects two __sorted__ arrays|`src1`, `src2`|Intersects arrays `src1` and `src2`, returning intersection|

<br/>

## Later
|Command|Purpose|Args|Notes|
|---|---|---|---|
|ARR_COPY|Copy array to new array|`src_name`,`dst_name`, `src_start`, `src_stop`|Creates `dst_name` array of size `src_stop - src_start` then copies from `[src_start, src_stop)`|
|ARR_MOVE|Move to a different array|`src_name`, `dst_name`|Create `dst_name` if required. No items copied. Does `std::move()`. `dst_name` array is cleared if it has items.|


<br/>

# List
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


<br/>

# Queue
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


<br/>

# Set
|Command|Purpose|Args|Notes|
|---|---|---|---|
|SET_CREATE|Create|`name`||
|SET_DELETE|Delete|`name`||
|SET_ADD|Add|`name`||
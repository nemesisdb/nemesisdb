---
sidebar_position: 4
displayed_sidebar: clientApisSidebar
sidebar_label: session_exists
---

# session_exists
Given an array of tokens, returns those that exist.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|tkns|List[int]|Tokens to check|Y|


## Returns

`tuple(bool, int)`
- `bool` : `True` is the command was successful, otherwise `False`
- `int` : the number of sessions ended


## Examples


```py
tokens = [123,456,789]
(ok, exist) = await client.exists(tokens)
```

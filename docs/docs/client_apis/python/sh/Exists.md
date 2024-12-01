---
sidebar_position: 4
displayed_sidebar: clientApisSidebar
sidebar_label: sh_session_exists
---

# sh_session_exists
Given an array of tokens, returns those that exist.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|tkns|List[int]|Tokens to check|Y|


## Returns
`List[int]` : the tokens that exist. Each token in the `tkns` parameter that exist will be returned


## Examples


```py
tokens = [123,456,789]
exist = await client.exists(tokens)
```

- Illustrative purposes only, a token won't have such a small value
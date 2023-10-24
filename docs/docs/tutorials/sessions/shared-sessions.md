---
sidebar_position: 4
---

# Shared Session
A shared session does not relate to authentication - a session can be accessed by any client with the session token.

A normal (non-shared) session token is essentially randomly generated, so if two clients create a session with the same name, they will receive a different token.

But there are situations where multiple clients want to share the same session, but how can they without knowing the session token? The only way would be to distribute the token between clients but this is inefficient and prone to error.

This is what a shared session is for.


## Shared Session Use Case
We have a platform that has web and mobile app users. We need default settings for both types in a session that never expires. The backend services that handle web and mobile users are completely separate, perhaps different micro services. We need a practical way to access default data without bothering the primary database, hardcoding values or using config files.





1. On initial deployment we use `SH_NEW` to create a session with `shared:true` and no `expiry` settings (never expires)

<center>

![SH_NEW](img/shared_shnew.png)

</center>

<br/>

2. Services that require access use `SH_OPEN` with the session name

<center>

![SH_NEW](img/shared_shopen.png)

</center>

3. Clients have the shared session's token which they use in subsequent commands

<br/>
<br/>

## Shared Session Token Generation
When a shared session is created with `SH_NEW` the token is generated from the session name. This ensures when `SH_OPEN` is used with the same name, the same token is returned.


:::important
A session opened with `SH_OPEN` is not restricted - i.e. data be deleted or update by any client with the token.

This is because NemesisDB doesn't authenticate users yet, which is required to determine if a client has permission to perform an action.
:::

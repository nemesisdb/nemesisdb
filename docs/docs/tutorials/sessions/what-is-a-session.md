---
sidebar_position: 1
---

# What is a Session
A session is just a data container. They are called sessions because data is intended to be stored for a period of time before being deleted:

- A session can expire
- When a session expires, all its data is deleted
- The session and data can be deleted or just the data. If you only delete data, you can continue to use the session

A session can represent:
  - Whilst a user is logged into your app
  - Whilst you're waiting for a user to confirm a One Time Password
  - Until the session data has been stored in the primary database


For example, when writing a service which handles users, you can create a separate session for each user that logs in and delete the session when they log out.

![](img/sessions_overview.png)

If user 2 logs out, their session can be ended which deletes their data, without affecting any other sessions. Session can also be set to expire with a duration. In this example,
a session expire could be used as an auto logout feature.

<br/>

## Session Data
Each session has a dedicated data store. When you store, get, update etc session data, you are only accessing that particular session's data store.

When you are finished with the data you can end the session. If the session was created with an expiry duration, the session will be automatically ended.

<br/>

## Session Token
A session is identified by a token, which is just a string. When you create a session the server returns a token which is used with commands to access the data.

<br/>

## Shared Sessions
A session doesn't belong to a particular client via some authentication process - it can be accessed by any client with the token. But there may be cases where sharing a session is useful, for example a session
that stores default settings which are common to many areas of the backend services. 

You could distribute the session token amongst all clients, but this adds complexity. This is what shared sessions are for.

When a session is shared, the session token can be retrieved from just the session name.

<details>
  <summary>Token Generation</summary>
  <div>
    <div>
      If a session is not shared:<br/>
      <ul>
        <li>The session name does not take part in token generation. This means two sessions with the same name do not generate the same token</li>
      </ul>
      If a session is shared:<br/>
      <ul>
        <li>The session name is used in token generation, allowing others to get the token from session name</li>
      </ul>
        
    </div>   
  </div>
</details>


<br/>
<hr/>


<br/>

# Example - Pizza Delivery
A pizza delivery app must track pizzas as they are out for delivery, but once delivered, the delivery data can be discarded.

1. When a pizza is out the oven and assigned to a driver, start a new session
2. Update the session data as the driver delivers (position, estimated time, etc)
3. When a pizza is delivered, the session is ended, deleting the data

An alternative is to have a session per driver. When a pizza is delivered its key is deleted from the session. The session ends when the driver finishes their shift.

<br/>


# Example - Shopping Basket
An ecommerce website lets customers add items to their basket, but many customers are just 'window shopping' - they add items to their basket without any intention to actually buy the products.

1. Create a session when a customer visits the site (or adds their first item)
2. Store each basket item in the session
3. Set the session expiry duration to a reasonable time to allow delays and decisions
4. If the customer leaves the site without completing the order, the basket will be cleared when the sessions expires


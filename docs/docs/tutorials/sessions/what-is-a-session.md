---
sidebar_position: 1
---

# What is a Session
When you store data in NemesisDB, it must be stored in a session. A session is used to separate data, for example:

- For a website, create a session for each user that logs in. The session ends after a set duration
- For a delivery app, create a session for each active delivery, which ends when the customer receives the package

A session has two required properties:

|Property|Meaning|
|:---|:---|
|Name| A session name. May be used when creating session token for shared sessions|
|Token| The session token. A session identifier |

It also has optional `expiry` properties:

|Property|Meaning|
|:---|:---|
|Duration| Time in seconds until the session expires |
|DeleteSession| Flag indicating if the session should be deleted. If `false`, only the data is deleted. |

<br/>

When a session ends its data is deleted. In some cases, you may want to delete the data but not the session, so the `deleteSession` setting controls this.

:::info
Sessions offer control of data availibility and memory consumption. 
:::

<br/>

# Example - Pizza Delivery
A pizza delivery app must track pizzas as they are out for delivery, but once delivered, the delivery data can be discarded.

1. When a pizza is marked as 'Ready for Delivery', start a new session
2. Update the session data as the driver delivers (position, estimated time, etc)
3. As each pizza is marked as 'Delivered', you can end the session which deletes the data

An alternative is to have a session per driver. When a pizza is delivered its key is deleted from the session. The session ends when the driver finishes their shift.

<br/>


# Example - Shopping Basket
An ecommerce website lets customers add items to their basket, but many customers are just 'window shopping' - they add items to their basket without any intention to actually buy the products.

1. Create a session when a customer visits the site (or adds their first item)
2. Store each basket item in the session
3. Set the session expiry duration to reasonable time to allow delays and decisions
4. If the customer leaves the site without completing the order, the basket will be cleared when the sessions expires




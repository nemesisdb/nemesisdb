import common
from typing import Tuple
from ndb.sessionclient import SessionClient, Session

import asyncio as asio
import random


"""Simulate One Time Password using expiring session.
- Create a session which expires
- Session contains the password
- Easily manage if user confirms before expiry

The session is deleted by the server when it expires, avoiding
client code having to do so manually.
"""

async def create_otp(client: SessionClient) -> Tuple[Session, int]:
  # create a session with expiry and session delete  
  session = await client.create_session(durationSeconds=2, deleteSessionOnExpire=True)
  
  # set the passcode, have expiry short for this example
  code = random.randint(1000, 9999)  
  await client.set({'code':code}, session.tkn)
  
  return (session, code)


async def validate_otp(client: SessionClient, tkn: int, userCode: int) -> bool:
  # if session doesn't exist, get() returns (False, dict()), otherwise check code
  print(f'Attempting {userCode}')
  (valid, result) = await client.get(('code',), tkn)
  return valid and result['code'] == userCode
  

async def otp():
  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  # this stores the OTP and would send to user's email/phone etc
  (session, code) = await create_otp(client)

  print(f"Correct Code {code}")

  # some time later user submits a code
  # but we simulate this in a loop

  valid = False
  maxAttempts = 5
  useCorrectCodeIndex = random.randint(0, maxAttempts-1)

  for i in range(0, maxAttempts):
    attemptCode = code if i == useCorrectCodeIndex else random.randint(1000, 9999)
    if (valid := await validate_otp(client, session.tkn, attemptCode)) == True:
      break
    else:
      await asio.sleep(1)

  print('Ok' if valid else 'Fail')


if __name__ == "__main__":
  asio.run(otp())

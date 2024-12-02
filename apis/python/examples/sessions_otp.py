from typing import Tuple
import sys
sys.path.append('../')
from ndb.client import NdbClient, Session

import asyncio as asio
import random


"""Simulate One Time Password using expiring session.
- Create a session which expires
- Session contains the password
- Easily manage if user confirms before expiry

The session is deleted by the server when it expires, avoiding
client code having to do so manually.
"""

async def create_otp(client: NdbClient) -> Tuple[Session, int]:
  # create a session with expiry and session delete  
  session = await client.sh_create_session(durationSeconds=2, deleteSessionOnExpire=True)
  
  # set the passcode, have expiry short for this example
  code = random.randint(1000, 9999)  
  await client.sh_set({'code':code}, session.tkn)
  return (session, code)


async def validate_otp(client: NdbClient, tkn: int, userCode: int) -> bool:
  # if session doesn't exist, get() returns (False, dict()), otherwise check code
  print(f'Attempting {userCode}')
  result = await client.sh_get(('code',), tkn)
  return result['code'] == userCode
  

async def otp():
  client = NdbClient()
  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return

  # this stores the OTP and would send to user's email/phone etc
  (session, code) = await create_otp(client)

  print(f"Correct Code {code}")

  # some time later user submits a code
  # simulate this in a loop, attempting the correct code
  # on attempt 'userCorrectCodeIndex', otherwise random code

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

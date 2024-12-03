import asyncio as asio
from os import path
from typing import List
import sys
import json
sys.path.append('../')
from ndb.client import NdbClient, Session


"""
This emulates using NemesisDB as a cache to authenticate users in a web app.

A database class used to emulate db calls. 
A cookie class used to emulate browser cookies (writes session tokens to file).

create user:
  - Store email as key, mapped to their password
  - When user logins, we can auth against the email/password
  - user login (assumes they'd login after creating account)

user login
  - create session, containing UI settings

user log out:
  - delete session

user delete:
  - user log out (deletes session)
  - remove from db
  - delete email from keys
"""


# class Database:
#   def createUser(self, email: str, password: str):
#     print(f'Database: create user: {email}')
#     return {'theme':'light', 'layout':'grid'}

#   def getUserSettings(self, email: str) -> dict:
#     print(f'Database: get user settings: {email}')
#     return {'theme':'light', 'layout':'grid'}
  
#   def userAuth(self, email: str, password: str) -> bool:
#     print(f'Database: user auth: {email}')
#     return True
  
#   def updateUi(self, email: str, settings: dict):
#     print(f'Database: user {email} settings update: {settings}')


# class Cookies:
#   ''' Simulate browser cookie with json file '''
#   cookieFilePath = "./cookies.json"
#   users = list()


#   def __init__(self):
#     self._readCookies()


#   def addCookie(self, token: int, email: str, password: str):
#     self.users = list()

#     if path.exists(self.cookieFilePath):
#       with open(self.cookieFilePath, 'r') as file:
#         content = json.load(file)
#         self.users = list(content['users'])
    
#     self.users.append({'token':token})

#     doc = {'users':self.users}

#     with open(self.cookieFilePath, 'w') as file:
#       json.dump(doc, file)


#   def _readCookies(self) -> List[int]:
#     self.users.clear()

#     if path.exists(self.cookieFilePath):
#       with open(self.cookieFilePath, 'r') as file:
#         content = json.load(file)
#         self.users = list(content['users'])



# database = Database()
# ndb = NdbClient()
# cookies = Cookies()


# async def create_user_session(sessionData: dict) -> int:
#   session = await ndb.sh_create_session(durationSeconds=1800, extendOnSetAdd=True, extendOnGet=True)
#   await ndb.sh_set(sessionData, session.tkn)
#   return session.tkn
    

# async def user_reauth(token: int):
#   exists = await ndb.sh_session_exists([token])
#   return token in exists


# # First login or previous session expired (no cookie)
# async def user_auth (email, password) -> int:
#   # check cache if user exists
#   info = await ndb.kv_get(key=email)
#   if len(info):
#     if info['password'] == password:
#       uiSettings = database.getUserSettings(email)
#       sessionData = {'email':email, 'password':password, 'ui':uiSettings}
#       tkn = await create_user_session(sessionData)
#       cookies.addCookie(tkn, email, password)
#       print(f'User f{email} authenticated, with token {tkn}')
#       return tkn
#   else:
#     return None
  


# async def user_create(email, password) -> Session:
#   # add user to db, which also returns default settings
#   uiSettings = database.createUser(email, password)
#   sessionData = {'email':email, 'password':password, 'ui':uiSettings}

#   # we key on the email since it will be unique, with value as an object
#   # containing the password so future logins can use the cache
#   await ndb.kv_set({email:{'password':password}})

#   tkn = await create_user_session(sessionData)
#   cookies.addCookie(tkn, email, password)

#   print(f'Created user {email} with token {tkn}')



# async def update_ui_settings(token: int, settings: dict):  
#   #session = await ndb.sh_get(('email',), token)
#   #email = session['email']
#   email = await ndb.sh_get(token, key='email')

#   print(f'Update UI for session {token} -> {email}')

#   database.updateUi(email, settings)
#   await ndb.sh_set({'ui':settings}, token)



# async def run():
#   await ndb.open('ws://127.0.0.1:1987')

#   # previous session expired or first login
#   authed = await user_auth('u1@e.com', 'u1_password')
#   print('Authed' if authed else 'Failed Auth')


#   # user has a cookie (previous session not expired)
#   # for user in cookies.users:
#   #   authed = await user_reauth(user['token'])
#   #   print('Authed' if authed else 'Failed: sesison expired. Must login again')

#   # create users
#   # user1 = ('u1@e.com', 'u1_password')
#   # token = await user_create(user1[0], user1[1])
   

#   # # update UI settings
#   # if len(cookies.users):
#   #   await update_ui_settings(cookies.users[0]['token'], {'theme':'dark', 'layout':'rows'})

# if __name__ == "__main__":
#   asio.run(run())


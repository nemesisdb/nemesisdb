# import asyncio as asio
# import sys
# sys.path.append('../')
# from ndb.kvclient import KvClient
# from ndb.sessionclient import SessionClient, Session


# class Database:
#   def newUser(email: str, password: str):
#     print(f'Database store new user: {email} , password: {password}')
#     return {'ui_theme':'light'}
  

# database = Database()
# ndb_sessions = SessionClient()
# ndb_kv = KvClient()


# async def create_new_user_session(email) -> dict:
#   defaultSettings = database.newUser(email)
#   user = {'email':email,
#           'ui_theme':defaultSettings['ui_theme']}

#   session = ndb_sessions.create_session(durationSeconds=1800, extentOnSetAdd=True, extendOnGet=True)
    


# async def create_existing_user_session(email) -> dict:
#   pass
  


# async def user_login (email, password) -> Session:
#   info = await ndb_kv.contains((email,))

#   if len(info):
#     userInfo = await ndb_kv.get((email,))
#     session = session(userInfo['tkn'])
#   else:
#     session = create_new_user_session()
#     set({email:{'tkn':session.tkn}})
    
#   return session


# async def run():
#   await ndb_sessions.open('ws://127.0.0.1:1987')
#   await ndb_kv.open('ws://127.0.0.1:1987')

#   user_login('u1@e.com')




# if __name__ == "__main__":
#   asio.run(run())


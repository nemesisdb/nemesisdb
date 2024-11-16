import sys
import asyncio as asio

sys.path.append('../')
import ndb
from ndb import SessionClient



async def create():
  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  session = await ndb.create_session(client)
  assert session != None and session.tkn > 0

  await client.close()


async def set_get():
  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  session = await ndb.create_session(client)
  assert session != None and session.tkn > 0

  assert await client.set({'fname':'james', 'sname':'smith'}, session.tkn)
  (ok, values) = await client.get(('fname', 'sname'), session.tkn)
  assert (ok and len(values) == 2 and
          values == {'fname':'james', 'sname':'smith'})

  await client.close()



async def exists_end():
  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  session = await ndb.create_session(client)
  assert session != None and session.tkn > 0

  [ok, exist] = await ndb.session_exists(client, session, [session.tkn])
  assert ok and exist == [session.tkn]
  
  assert (await ndb.end_session(client, session))

  [ok, exist] = await ndb.session_exists(client, session, [session.tkn])
  assert ok and len(exist) == 0

  await client.close()


async def end_all():
  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  max = 10

  session = await ndb.create_session(client)
  assert session != None and session.tkn > 0

  [ok, count] = await ndb.end_all_sessions(client)
  # count>0 because we dont know how many sessions exist
  # from other tests
  assert ok and count > 0 


async def end_all_multiple():
  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  max = 10
  sessions = list()

  for i in range(0, max):
    session = await ndb.create_session(client)
    assert session != None and session.tkn > 0
    sessions.append(session)

  [ok, count] = await ndb.end_all_sessions(client)
  assert ok and count == max
  


async def info_infoall():
  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  # end all so we know the numbers
  [ok, _] = await ndb.end_all_sessions(client)
  assert ok 


  session = await ndb.create_session(client)
  assert session != None and session.tkn > 0

  assert (await client.set({'fname':'james', 'sname':'smith'}, session.tkn))

  (ok, result) = await ndb.session_info(client, session)
  assert (ok and result['tkn'] == session.tkn and
                 result['keyCnt'] == 2 and
                 result['shared'] == False and
                 result['expires'] == False)


  (ok, result) = await ndb.session_info_all(client, session)
  assert (ok and result['totalSessions'] == 1 and result['totalKeys'] == 2)


   

if __name__ == "__main__":
  for f in [create(), set_get(), exists_end(), end_all(), end_all_multiple(), info_infoall()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)

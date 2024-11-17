import sys
import random
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

  [ok, exist] = await ndb.session_exists(session, [session.tkn])
  assert ok and exist == [session.tkn]
  
  assert (await ndb.end_session(session))

  [ok, exist] = await ndb.session_exists(session, [session.tkn])
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

  (ok, result) = await ndb.session_info(session)
  assert (ok and result['tkn'] == session.tkn and
                 result['keyCnt'] == 2 and
                 result['shared'] == False and
                 result['expires'] == False)


  (ok, result) = await ndb.session_info_all(session)
  assert (ok and result['totalSessions'] == 1 and result['totalKeys'] == 2)


async def create_expires(waitForExpiry = False):
  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  serverTimerPeriod = 5 # period of the server's timer which handles expiry
  sessionDuration = 2

  session = await ndb.create_session(client, durationSeconds=sessionDuration)
  assert session != None and session.tkn > 0

  await client.set({'k1':'v1', 'k2':'v2', 'k3':'v3'}, session.tkn)

  (ok, result) = await ndb.session_info(session)
  assert (ok and result['tkn'] == session.tkn and
                 result['keyCnt'] == 3 and
                 result['shared'] == False and
                 result['expires'] == True and
                 result['expiry']['duration'] == sessionDuration and
                 result['expiry']['deleteSession'] == False)

   
  if waitForExpiry:
    # wait for expiry then confirm keys is 0
    # must sleep long enough to ensure server has fired the timer
    await asio.sleep(max(sessionDuration+1, serverTimerPeriod+sessionDuration))
    (ok, result) = await ndb.session_info(session)
    assert ok and result['keyCnt'] == 0


async def save_load_one_session():
  dataSetName = f'test_{random.randint(0, 10000)}'

  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  # clear before start
  (cleared, count) = await ndb.end_all_sessions(client)
  assert cleared
  
  session = await ndb.create_session(client)
  assert session != None and session.tkn > 0

  assert await client.set({'fname':'james', 'sname':'smith'}, session.tkn)

  assert await ndb.save_session(session, dataSetName)

  # clear and load
  (cleared, count) = await ndb.end_all_sessions(client)
  assert cleared and count == 1

  (loaded, rsp) = await ndb.load_session(client, dataSetName)
  assert loaded and rsp['sessions'] == 1 and rsp['keys'] == 2
  


async def save_load_all_sessions(nSessions: int, nKeysPerSession: int):
  dataSetName = f'test_{random.randint(0, 10000)}'

  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  # clear before start
  (cleared, count) = await ndb.end_all_sessions(client)
  assert cleared


  # create sessions, set two keys per session
  for _ in range(0, nSessions):
    session = await ndb.create_session(client)
    assert session != None and session.tkn > 0
    for k in range(0,nKeysPerSession):
      assert await client.set({f'key{k}':'some_value'}, session.tkn)
  

  # leave 'tkns' arg empty to save all sessions
  assert await ndb.save_sessions(client, dataSetName)

  # clear then load
  (cleared, count) = await ndb.end_all_sessions(client)
  assert cleared and count == nSessions

  (loaded, rsp) = await ndb.load_session(client, dataSetName)
  assert loaded and rsp['sessions'] == nSessions and rsp['keys'] == nSessions*nKeysPerSession
  



async def save_load_select_sessions(nSessions: int, nKeysPerSession: int):
  dataSetName = f'test_{random.randint(0, 10000)}'

  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  # clear before start
  (cleared, count) = await ndb.end_all_sessions(client)
  assert cleared


  tokensToSave = list()

  # create sessions
  for s in range(0, nSessions):
    session = await ndb.create_session(client)
    assert session != None and session.tkn > 0
    for k in range(0,nKeysPerSession):
      assert await client.set({f'key{k}':'some_value'}, session.tkn)
    
    # store list of tokens to save
    if s % 2 == 0:
      tokensToSave.append(session.tkn)


  assert await ndb.save_sessions(client, dataSetName, tokensToSave)

  # clear and then load
  (cleared, count) = await ndb.end_all_sessions(client)
  assert cleared and count == nSessions

  (loaded, rsp) = await ndb.load_session(client, dataSetName)
  assert loaded and rsp['sessions'] == len(tokensToSave) and rsp['keys'] == len(tokensToSave)*nKeysPerSession

  


if __name__ == "__main__":
  tests = [
            create(), create_expires(False), set_get(), exists_end(),
            end_all(), end_all_multiple(), info_infoall(),
            save_load_one_session(), save_load_all_sessions(100, 100),
            save_load_select_sessions(100,5)
          ]

  for f in tests:
    print(f'---- {f.__name__} ----')
    asio.run(f)

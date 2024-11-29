import sys
import random
import asyncio as asio

sys.path.append('../')
import ndb
from ndb.sessionclient import SessionClient



async def create():
  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  session = await client.create_session()
  assert session.isValid

  await client.close()


async def set_get():
  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  session = await client.create_session()
  assert session.isValid

  assert await client.set({'fname':'james', 'sname':'smith'}, session.tkn)
  (ok, values) = await client.get(('fname', 'sname'), session.tkn)
  assert (ok and len(values) == 2 and
          values == {'fname':'james', 'sname':'smith'})

  await client.close()


async def exists_end():
  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  session = await client.create_session()
  assert session.isValid

  [ok, exist] = await client.session_exists([session.tkn])
  assert ok and exist == [session.tkn]
  
  assert (await client.end_session(session.tkn))

  [ok, exist] = await client.session_exists([session.tkn])
  assert ok and len(exist) == 0

  await client.close()


async def end_all():
  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  max = 10

  session = await client.create_session()
  assert session.isValid

  [ok, count] = await client.end_all_sessions()
  # count>0 because we dont know how many sessions exist
  # from other tests
  assert ok and count > 0 


async def end_all_multiple():
  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  max = 10
  sessions = list()

  for i in range(0, max):
    session = await client.create_session()
    assert session.isValid
    sessions.append(session)

  [ok, count] = await client.end_all_sessions()
  assert ok and count == max
  

async def info_infoall():
  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  # end all before we begin
  [ok, _] = await client.end_all_sessions()
  assert ok 


  session = await client.create_session()
  assert session.isValid

  assert (await client.set({'fname':'james', 'sname':'smith'}, session.tkn))

  (ok, result) = await client.session_info(session.tkn)
  assert (ok and result['tkn'] == session.tkn and
                 result['keyCnt'] == 2 and
                 result['shared'] == False and
                 result['expires'] == False)


  (ok, result) = await client.session_info_all()
  assert (ok and result['totalSessions'] == 1 and result['totalKeys'] == 2)


async def create_expires(waitForExpiry = False):
  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  serverTimerPeriod = 5 # period of the server's timer which handles expiry
  sessionDuration = 2

  session = await client.create_session(durationSeconds=sessionDuration)
  assert session.isValid

  await client.set({'k1':'v1', 'k2':'v2', 'k3':'v3'}, session.tkn)

  (ok, result) = await client.session_info(session.tkn)
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
    (ok, result) = await client.session_info(session.tkn)
    assert ok and result['keyCnt'] == 0


async def save_load_one_session():
  dataSetName = f'test_{random.randint(0, 10000)}'

  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  # clear before start
  (cleared, count) = await client.end_all_sessions()
  assert cleared
  
  session = await client.create_session()
  assert session.isValid

  assert await client.set({'fname':'james', 'sname':'smith'}, session.tkn)
  assert await client.save(dataSetName, [session.tkn])

  # clear and load
  (cleared, count) = await client.end_all_sessions()
  assert cleared and count == 1

  (loaded, rsp) = await client.load(dataSetName)
  assert loaded and rsp['sessions'] == 1 and rsp['keys'] == 2
  

async def save_load_all_sessions(nSessions: int, nKeysPerSession: int):
  dataSetName = f'test_{random.randint(0, 10000)}'

  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  # clear before start
  (cleared, count) = await client.end_all_sessions()
  assert cleared


  # create sessions, set two keys per session
  for _ in range(0, nSessions):
    session = await client.create_session()
    assert session.isValid
    for k in range(0,nKeysPerSession):
      assert await client.set({f'key{k}':'some_value'}, session.tkn)
  

  # leave 'tkns' empty to save all sessions
  assert await client.save(dataSetName)

  # clear then load
  (cleared, count) = await client.end_all_sessions()
  assert cleared and count == nSessions

  (loaded, rsp) = await client.load(dataSetName)
  assert loaded and rsp['sessions'] == nSessions and rsp['keys'] == nSessions*nKeysPerSession
  

async def save_load_select_sessions(nSessions: int, nKeysPerSession: int):
  dataSetName = f'test_{random.randint(0, 10000)}'

  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  # clear before start
  (cleared, count) = await client.end_all_sessions()
  assert cleared


  tokensToSave = list()

  # create sessions
  for s in range(0, nSessions):
    session = await client.create_session()
    assert session.isValid
    for k in range(0,nKeysPerSession):
      assert await client.set({f'key{k}':'some_value'}, session.tkn)
    
    # store list of tokens to save
    if s % 2 == 0:
      tokensToSave.append(session.tkn)


  assert await client.save(dataSetName, tokensToSave)

  # clear and then load
  (cleared, count) = await client.end_all_sessions()
  assert cleared and count == nSessions

  (loaded, rsp) = await client.load(dataSetName)
  assert loaded and rsp['sessions'] == len(tokensToSave) and rsp['keys'] == len(tokensToSave)*nKeysPerSession

  


if __name__ == "__main__":
  tests = [
            create(), create_expires(False), set_get(), exists_end(),
            end_all(), end_all_multiple(), info_infoall(),
            save_load_one_session(), save_load_all_sessions(50, 100),
            save_load_select_sessions(50,5)
         ]

  for f in tests:
    print(f'---- {f.__name__} ----')
    asio.run(f)

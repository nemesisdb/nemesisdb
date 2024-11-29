import sys
import random
import asyncio as asio

sys.path.append('../')
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

  await client.set({'fname':'james', 'sname':'smith'}, session.tkn)
  values = await client.get(('fname', 'sname'), session.tkn)
  assert len(values) == 2 and values == {'fname':'james', 'sname':'smith'}

  await client.close()


async def end_all():
  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  await client.end_all_sessions()

  max = 10

  for _ in range(0, max):
    session = await client.create_session()
    assert session.isValid
  
  count = await client.end_all_sessions()
  assert count == max 


async def exists_end():
  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  session = await client.create_session()
  assert session.isValid

  exist = await client.session_exists([session.tkn])
  assert exist == [session.tkn]
  
  await client.end_session(session.tkn)

  exist = await client.session_exists([session.tkn])
  assert len(exist) == 0

  await client.close()
  

async def info_infoall():
  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  # end all before we begin
  await client.end_all_sessions()

  session = await client.create_session()
  assert session.isValid

  await client.set({'fname':'james', 'sname':'smith'}, session.tkn)

  result = await client.session_info(session.tkn)
  assert (result['tkn'] == session.tkn and
          result['keyCnt'] == 2 and
          result['shared'] == False and
          result['expires'] == False)


  result = await client.session_info_all()
  assert (result['totalSessions'] == 1 and result['totalKeys'] == 2)


async def create_expires(waitForExpiry = False):
  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  serverTimerPeriod = 5 # period of the server's timer which handles expiry
  sessionDuration = 2

  session = await client.create_session(durationSeconds=sessionDuration)
  assert session.isValid

  await client.set({'k1':'v1', 'k2':'v2', 'k3':'v3'}, session.tkn)

  result = await client.session_info(session.tkn)
  assert (result['tkn'] == session.tkn and
          result['keyCnt'] == 3 and
          result['shared'] == False and
          result['expires'] == True and
          result['expiry']['duration'] == sessionDuration and
          result['expiry']['deleteSession'] == False)

   
  if waitForExpiry:
    # wait for expiry then confirm keys is 0
    # must sleep long enough to ensure server has fired the timer
    await asio.sleep(max(sessionDuration+1, serverTimerPeriod+sessionDuration))
    result = await client.session_info(session.tkn)
    assert result['keyCnt'] == 0


async def save_load_one_session():
  dataSetName = f'test_{random.randint(0, 10000)}'

  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  # clear before start
  count = await client.end_all_sessions()
  
  session = await client.create_session()
  assert session.isValid

  await client.set({'fname':'james', 'sname':'smith'}, session.tkn)
  await client.save(dataSetName, [session.tkn])

  # clear and load
  count = await client.end_all_sessions()
  assert count == 1

  rsp = await client.load(dataSetName)
  assert rsp['sessions'] == 1 and rsp['keys'] == 2
  

async def save_load_all_sessions(nSessions: int, nKeysPerSession: int):
  dataSetName = f'test_{random.randint(0, 10000)}'

  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  # clear before start
  count = await client.end_all_sessions()

  # create sessions, set two keys per session
  for _ in range(0, nSessions):
    session = await client.create_session()
    assert session.isValid
    for k in range(0,nKeysPerSession):
      await client.set({f'key{k}':'some_value'}, session.tkn)
  

  # leave 'tkns' empty to save all sessions
  await client.save(dataSetName)

  # clear then load
  count = await client.end_all_sessions()
  assert count == nSessions

  rsp = await client.load(dataSetName)
  assert rsp['sessions'] == nSessions and rsp['keys'] == nSessions*nKeysPerSession
  

async def save_load_select_sessions(nSessions: int, nKeysPerSession: int):
  dataSetName = f'test_{random.randint(0, 10000)}'

  client = SessionClient()
  await client.open('ws://127.0.0.1:1987/')

  # clear before start
  count = await client.end_all_sessions()


  tokensToSave = list()

  # create sessions
  for s in range(0, nSessions):
    session = await client.create_session()
    assert session.isValid
    for k in range(0,nKeysPerSession):
      await client.set({f'key{k}':'some_value'}, session.tkn)
    
    # store list of tokens to save
    if s % 2 == 0:
      tokensToSave.append(session.tkn)


  await client.save(dataSetName, tokensToSave)

  # clear and then load
  count = await client.end_all_sessions()
  assert count == nSessions

  rsp = await client.load(dataSetName)
  assert rsp['sessions'] == len(tokensToSave) and rsp['keys'] == len(tokensToSave)*nKeysPerSession

  

if __name__ == "__main__":
  
  tests = [ 
            create(),
            create_expires(False),
            set_get(),
            end_all(),
            exists_end(),
            info_infoall(),
            save_load_one_session(),
            save_load_all_sessions(50, 100),
            save_load_select_sessions(50,5)
        ]
  
  for f in tests:
    print(f'---- {f.__name__} ----')
    asio.run(f)

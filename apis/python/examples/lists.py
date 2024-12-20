import asyncio as asio
import sys
sys.path.append('../')
from pprint import pprint

from ndb.client import NdbClient, ResponseError
from ndb.lists import ObjLists


async def delete_all(client: NdbClient):
  await ObjLists(client).delete_all()


async def olst_as_queue():
  """Uses list as a queue to track players waiting to join a busy gaming server. Players are added to the
  to the tail unless they have a special 'Queue Skip' perk, in which case they are added to the head.

  This is contrived, it'd be best to have separate queues for those normal and queue skip players.
  """
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  await delete_all(client)
  
  players = [
              {'name':'Batman', 'avg_ping':32},
              {'name':'Leo', 'avg_ping':28},
              {'name':'Lara', 'avg_ping':35}
            ]
  
  lists = ObjLists(client)

  await lists.create('join_queue')
  
  await lists.add('join_queue', players)

  # new player is allowed to skip queue, so add to front
  await lists.add_head('join_queue', {'name':'Witcher', 'avg_ping':20})

  # print queue
  print ('Queue:')
  for player in await lists.get_rng('join_queue', start=0):
    print(player)

  # server lets 2 players join so remove them from queue
  remaining = await lists.remove('join_queue', start=0, stop=2)
  print(f'Now {remaining} players waiting')

  # print remaining
  print ('Queue:')
  for player in await lists.get_rng('join_queue', start=0):
    print(player)



async def olst_as_stack():
  """
  How to use a list as a stack for JSON objects.
  """
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  await delete_all(client)

  # create API object
  lists = ObjLists(client)

  await lists.create('stack')

  print('Push')
  await lists.add_head('stack', {'name':'first'})
  await lists.add_head('stack', {'name':'second'})
  await lists.add_head('stack', {'name':'third'})
  
  for item in await lists.get_rng('stack', start=0):
    print(item)

  print('Pop')
  await lists.remove_head('stack')

  for item in await lists.get_rng('stack', start=0):
    print(item)



async def olst_create():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  await delete_all(client)

  lists = ObjLists(client)

  await lists.create('list1')
  await lists.create('list2')

  # add single item to list1
  await lists.add('list1', {'prod_name':'TV', 'qty':1})
  # add multiple items to list2
  await lists.add('list2', [{'prod_name':'Lamp', 'qty':2}, {'prod_name':'Chair', 'qty':3}])

  print(await lists.get_rng('list1', start=0))
  print(await lists.get_rng('list2', start=0))


async def olst_add():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  await delete_all(client)

  # create API object
  lists = ObjLists(client)

  await lists.create('my_list')
  # add an item
  await lists.add('my_list', {'username':'Bob'})
  # add multiple items. pos isn't set, so add to tail by default
  await lists.add('my_list', [{'username':'Brian'}, {'username':'Charles'}])
  # add multiple items to the head. can use add() with pos=0, or add_head()
  await lists.add_head('my_list', [{'username':'Alice'}, {'username':'Anna'}])
  
  
  names = await lists.get_rng('my_list', start=0)
  print(f'Before Brenda: {names}')

  # insert Brenda at Brian
  await lists.add('my_list', {'username':'Brenda'}, pos=3)

  names = await lists.get_rng('my_list', start=0)
  print(f'After: {names}')


if __name__ == "__main__":
  for f in [
              olst_as_queue(),
              olst_as_stack(),
              olst_create(),
              olst_add()
            ]:
    
    print(f'---- {f.__name__} ----')
    asio.run(f)
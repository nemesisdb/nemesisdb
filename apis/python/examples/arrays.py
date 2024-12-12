import asyncio as asio
import sys
sys.path.append('../')
from pprint import pprint

from ndb.client import NdbClient, ResponseError
from ndb.arrays import IntArrays, SortedIntArrays, ObjArrays, SortedStrArrays, StringArrays


async def delete_all(client: NdbClient):
  await IntArrays(client).delete_all()
  await ObjArrays(client).delete_all()
  await StringArrays(client).delete_all()
  await SortedStrArrays(client).delete_all()
  await SortedIntArrays(client).delete_all()


async def iarr_unsorted_set_rng():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  await delete_all(client)

  # create API object
  arrays = IntArrays(client)
  
  # create array called 'example' of length 4
  await arrays.create('example', 4)
  
  # set four integers
  await arrays.set_rng('example', [100,50,200,10], 0)
  
  # get all values: stop is exclusive
  allValues = await arrays.get_rng('example', start=0, stop=5)
  print(allValues)


async def iarr_sorted_set_rng():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  await delete_all(client)

  sortedArrays = SortedIntArrays(client)

  await sortedArrays.create('sorted', 4)

  # set four integers
  await sortedArrays.set_rng('sorted', [100,50,200,10])
  
  # get all values: stop is exclusive
  allValues = await sortedArrays.get_rng('sorted', start=0, stop=5)
  print(allValues)


async def iarr_create_sorted_unsorted():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  await delete_all(client)
  # create API object for unsorted and sorted int array
  unsortedArrays = IntArrays(client)
  sortedArrays = SortedIntArrays(client)

  # can use the same name because arrays are different type
  await unsortedArrays.create('my_array', 4)
  await sortedArrays.create('my_array', 4)

  await unsortedArrays.set_rng('my_array', [100,50,200,10])
  await sortedArrays.set_rng('my_array', [100,50,200,10])

  # omit 'stop', get all values
  unsortedValues = await unsortedArrays.get_rng('my_array', start=0)
  sortedValues = await sortedArrays.get_rng('my_array', start=0)

  print(unsortedValues)
  print(sortedValues)


async def arr_set_multiple_types():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  await delete_all(client)

  objectArrays = ObjArrays(client)
  sortedStrArrays = SortedStrArrays(client)
  sortedIntArrays = SortedIntArrays(client)

  await objectArrays.create('student_records', 3)
  await sortedStrArrays.create('student_names_sorted', 3)
  await sortedIntArrays.create('student_scores', 3)
  
  # student records as json objects
  await objectArrays.set('student_records', {'name':'Alice', 'modules':['Geography', 'Biology']})
  await objectArrays.set('student_records', {'name':'Bob', 'modules':['Art', 'Philosophy']})
  await objectArrays.set('student_records', {'name':'Charles', 'modules':['Music']})

  # sorted names
  await sortedStrArrays.set('student_names_sorted', 'Charles')
  await sortedStrArrays.set('student_names_sorted', 'Bob')
  await sortedStrArrays.set('student_names_sorted', 'Alice')

  # sorted leaderboard scores
  await sortedIntArrays.set('student_scores', 21)
  await sortedIntArrays.set('student_scores', 20)
  await sortedIntArrays.set('student_scores', 18)

  records = await objectArrays.get_rng('student_records', start=0)
  names = await sortedStrArrays.get_rng('student_names_sorted', start=0)
  scores = await sortedIntArrays.get_rng('student_scores', start=0)

  print('Records')
  pprint(records)
  print('Names')
  print(names)
  print('Scores')
  print(scores)


async def arr_setrng_multiple_types():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  await delete_all(client)

  objectArrays = ObjArrays(client)
  sortedStrArrays = SortedStrArrays(client)
  sortedIntArrays = SortedIntArrays(client) 

  await objectArrays.create('student_records', 3)
  await sortedStrArrays.create('student_names_sorted', 3)
  await sortedIntArrays.create('student_scores', 3)

  # student records as json objects
  records = [
              {'name':'Alice', 'modules':['Geography', 'Biology']},
              {'name':'Bob', 'modules':['Art', 'Philosophy']},
              {'name':'Charles', 'modules':['Music']}
            ]

  await objectArrays.set_rng('student_records', records)

  # sorted names
  await sortedStrArrays.set_rng('student_names_sorted', ['Charles', 'Bob', 'Alice'])
  # sorted leaderboard scores
  await sortedIntArrays.set_rng('student_scores', [21, 20, 18])

  records = await objectArrays.get_rng('student_records', start=0)
  names = await sortedStrArrays.get_rng('student_names_sorted', start=0)
  scores = await sortedIntArrays.get_rng('student_scores', start=0)

  print('Records')
  pprint(records)
  print('Names')
  print(names)
  print('Scores')
  print(scores)


async def iarr_get():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')
  await delete_all(client)

  values = [56,34,78,45]

  # unsorted
  unsortedInts = IntArrays(client)
  await unsortedInts.create('values', 4)
  await unsortedInts.set_rng('values', values)

  # sorted
  sortedInts = SortedIntArrays(client)
  await sortedInts.create('values', 4)
  await sortedInts.set_rng('values', values)

  first = await unsortedInts.get('values', 0)
  last = await unsortedInts.get('values', 3)
  print(f'Unsorted: {first} and {last}')

  first = await sortedInts.get('values', 0)
  last = await sortedInts.get('values', 3)
  print(f'Sorted: {first} and {last}')


async def iarr_get_rng():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')
  await delete_all(client)

  sortedInts = SortedIntArrays(client)
  await sortedInts.create('scores', 5)
  await sortedInts.set_rng('scores', [50,102,95,64,22])

  # get all and slice
  scores = await sortedInts.get_rng('scores', start=0)
  
  print(f'Ascending: {scores}')
  print(f'Descending: {scores[::-1]}')
  print(f'High: {scores[-1]}, '
        f'Low: {scores[0]}, '
        f'Top Three: {scores[-1:-4:-1]}')

  
  # get just the top three
  scores = await sortedInts.get_rng('scores', start=2)
  print(f'Top Three with get_rng(): {scores[::-1]}')
  

async def iarr_clear():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')
  await delete_all(client)

  arrays = IntArrays(client)
  await arrays.create('values', 4)

  # fill array
  await arrays.set_rng('values', [56,34,78,45])
  
  values = await arrays.get_rng('values', start=0)
  print(values)

  # array is full
  try:
    await arrays.set_rng('values', [99, 66])
  except ResponseError:
    print("Array full")

  # clear 34 and 78 then try again
  await arrays.clear('values', start=1, stop=3)
  await arrays.set_rng('values', [99,66])
  
  values = await arrays.get_rng('values', start=0)
  print(values)


async def iarr_clear_all():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')
  await delete_all(client)

  arrays = IntArrays(client)
  await arrays.create('values', 4)

  # fill array
  await arrays.set_rng('values', [56,34,78,45])
  
  values = await arrays.get_rng('values', start=0)
  print(values)
  
  # clear all and set new
  await arrays.clear('values', start=0, stop=5)
  await arrays.set_rng('values', [33,11,99,66])
  
  values = await arrays.get_rng('values', start=0)
  print(values)


async def iarr_intersect():
  async def createData(s: int, size: int):
    from random import sample, seed
    seed(s)
    return sample(range(0, size*4), size)

  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')
  await delete_all(client)

  print('Creating data') 
  data1 = await createData(7,500)
  data2 = await createData(9,500)

  print('Creating arrays') 
  sortedInts = SortedIntArrays(client)
  await sortedInts.create('array1', 500)
  await sortedInts.create('array2', 500)
  
  print('Storing data')
  await sortedInts.set_rng('array1', data1)
  await sortedInts.set_rng('array2', data2)

  print('Intersecting')
  intersected = await sortedInts.intersect('array1', 'array2')
  print(f'Intersecting has: {len(intersected)} values')


async def sarr_delete():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')
  await delete_all(client)

  arrays = StringArrays(client)
  await arrays.create('names', 4)
  await arrays.set_rng('names', ['hello', 'world', 'goodbye', 'world'])

  await arrays.delete('names')
  # we can create an array with the same name after deleting the original
  await arrays.create('names', 3)
  await arrays.set_rng('names', ['hello', 'world', 'again'])


async def sarr_used():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')
  await delete_all(client)

  arrays = StringArrays(client)
  await arrays.create('values', capacity=4)
  
  # no items stored
  capacity = await arrays.capacity('values')
  used = await arrays.used('values')
  print (f'1. capacity: {capacity}, used: {used}')

  # store 3 items, leaving 1 array slot free
  await arrays.set_rng('values', ['a', 'b', 'c'])
  capacity = await arrays.capacity('values')
  used = await arrays.used('values')
  print (f'2. capacity: {capacity}, used: {used}')

  # clear the first 2 items
  await arrays.clear('values', start=0, stop=2)
  
  capacity = await arrays.capacity('values')
  used = await arrays.used('values')
  print (f'3. capacity: {capacity}, used: {used}')


if __name__ == "__main__":
  for f in [iarr_unsorted_set_rng(),
            iarr_sorted_set_rng(),
            iarr_create_sorted_unsorted(),
            arr_set_multiple_types(),
            arr_setrng_multiple_types(),
            iarr_get(),
            iarr_get_rng(),
            iarr_clear(),
            iarr_clear_all(),
            iarr_intersect(),
            sarr_delete(),
            sarr_used()]:
    
    print(f'---- {f.__name__} ----')
    asio.run(f)
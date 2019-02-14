#!/usr/bin/env python

valhalla_tiles = [{'level': 2, 'size': 0.25}, {'level': 1, 'size': 1.0}, {'level': 0, 'size': 4.0}]
LEVEL_BITS = 3
TILE_INDEX_BITS = 22
ID_INDEX_BITS = 21
LEVEL_MASK = (2**LEVEL_BITS) - 1
TILE_INDEX_MASK = (2**TILE_INDEX_BITS) - 1
ID_INDEX_MASK = (2**ID_INDEX_BITS) - 1
INVALID_ID = (ID_INDEX_MASK << (TILE_INDEX_BITS + LEVEL_BITS)) | (TILE_INDEX_MASK << LEVEL_BITS) | LEVEL_MASK

def get_tile_level(id):
  return id & LEVEL_MASK

def get_tile_index(id):
  return (id >> LEVEL_BITS) & TILE_INDEX_MASK

def get_index(id):
  return (id >> (LEVEL_BITS + TILE_INDEX_BITS)) & ID_INDEX_MASK

def tiles_for_bounding_box(left, bottom, right, top):
  #if this is crossing the anti meridian split it up and combine
  if left > right:
    east = tiles_for_bounding_box(left, bottom, 180.0, top)
    west = tiles_for_bounding_box(-180.0, bottom, right, top)
    return east + west
  #move these so we can compute percentages
  left += 180
  right += 180
  bottom += 90
  top += 90
  tiles = []
  #for each size of tile
  for tile_set in valhalla_tiles:
    #for each column
    for x in range(int(left/tile_set['size']), int(right/tile_set['size']) + 1):
      #for each row
      for y in range(int(bottom/tile_set['size']), int(top/tile_set['size']) + 1):
        #give back the level and the tile index
        tiles.append((tile_set['level'], int(y * (360.0/tile_set['size']) + x)))
  return tiles

def get_tile_id(tile_level, lat, lon):
  level = filter(lambda x: x['level'] == tile_level, valhalla_tiles)[0]
  width = int(360 / level['size'])
  return int((lat + 90) / level['size']) * width + int((lon + 180 ) / level['size'])

def get_ll(id):
  tile_level = get_tile_level(id)
  tile_index = get_tile_index(id)
  level = filter(lambda x: x['level'] == tile_level, valhalla_tiles)[0]
  width = int(360 / level['size'])
  height = int(180 / level['size'])
  return int(tile_index / width) * level['size'] - 90, (tile_index % width) * level['size'] - 180

from PIL import Image
import random
orig = Image.open('stars.png')
pmap = orig.load()

stars = []

out = Image.new(orig.mode, (orig.size[0] * 4, orig.size[1] * 2))
pmapout = out.load()

for x in range(orig.size[0]):
    for y in range(orig.size[1]):
      if pmap[x, y][0] > 0 or pmap[x, y][1] > 0 or pmap[x, y][2] > 0:
        stars.append([x, y, pmap[x, y], random.randint(0, 7)])


for frame in range(0, 7):
  for x in range(orig.size[0]):
      for y in range(orig.size[1]):
        pmapout[x + (frame % 4) * orig.size[0], y + int(frame / 4) * orig.size[1]] = (0, 0, 0)
        for star in stars:
          stFrame = (star[3] + frame) % 8
          tup = (int(star[2][0] / 2), int(star[2][1] / 2), int(star[2][2] / 2))
          if x == star[0] and y == star[1] and (stFrame == 3 or stFrame == 7):
            print(tup)
            pmapout[x + (frame % 4) * orig.size[0], y + int(frame / 4) * orig.size[1]] = tup
          if x == star[0] and y == star[1] and (stFrame == 5 or stFrame == 6):
            pmapout[x + (frame % 4) * orig.size[0], y + int(frame / 4) * orig.size[1]] = star[2]

orig.close()
out.save('stars-frame.png')
out.close()

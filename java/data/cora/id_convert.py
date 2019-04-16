#!/usr/bin/env python

from collections import defaultdict

id_map = dict()
label_map = dict()
writer = open('cora.content.id', 'w')
label_writer = open('cora.label', 'w')
feature_writer = open('cora.feature', 'w')

with open('cora.content') as fp:
  for i, line in enumerate(fp):
    info = line.strip().split()
    id_map[info[0]] = i
    label = info[-1]
    if label not in label_map:
      label_map[label] = len(label_map)
    label = label_map[label]
    
    feature = info[1:-1]
    indices = []
    for k, v in enumerate(feature):
      if int(v) != 0:
        indices.append(str(k))
    
    output = '%s %s %s\n' % (str(i), label, ' '.join(indices))
    writer.write(output)
    output = '%s %s\n' % (str(i), label)
    label_writer.write(output)
    output = '%s %s\n' % (str(i), ' '.join(indices))
    feature_writer.write(output)


edges = defaultdict(set)
with open('cora.cites') as fp:
  for line in fp:
    u, v = line.strip().split()
    u = id_map[u]
    v = id_map[v]
    # edges[u].add(v)
    edges[v].add(u)

writer = open('cora.adjs', 'w')
for u, vs in sorted(edges.items(), key=lambda x:x[0]):
  vs = [str(x) for x in vs]
  output = '%s %s\n' % (str(u), ' '.join(list(vs)))
  writer.write(output)

writer = open('id_map', 'w')
for u, v in sorted(id_map.items(), key=lambda x:x[1]):
  writer.write('%s %s\n' % (str(v), str(u)))

writer = open('label_map', 'w')
for u, v in sorted(label_map.items(), key=lambda x:x[1]):
  writer.write('%s %s\n' % (str(v), str(u)))

writer = open('cora.edge', 'w')
for u, vs in sorted(edges.items(), key=lambda x:x[0]):
    for v in vs:
        output = '%s %s\n' % (str(u), str(v))
        writer.write(output)
    

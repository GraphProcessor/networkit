
from _NetworKit import KatzIndex, CommonNeighborsIndex, JaccardIndex, PreferentialAttachmentIndex, AdamicAdarIndex, UDegreeIndex, VDegreeIndex, AlgebraicDistanceIndex, NeighborhoodDistanceIndex, TotalNeighborsIndex, NeighborsMeasureIndex, TrainingGraphSampler, ROCMetric, KFoldCrossValidator, PrecisionRecallMetric, MissingLinksFinder

from .graph import Graph

import numpy as np

try:
  import sklearn
except ImportError:
  print(""" WARNING: module 'sklearn' not found, link prediction functionality will be limited """)

def trainClassifier(trainingSet, trainingGraph, classifier, *linkPredictors):
  """
  Trains the given classifier with the feature-vectors generated by the given linkPredictors.

  Parameters
  ----------
  trainingSet : vector[pair[node, node]]
    Vector of node-pairs to generate features for,
  trainingGraph : Graph
    Training graph containg all edges from the training set.
  classifier:
    Scikit-learn classifier to train.
  linkPredictors:
    Predictors used for the generation of feature-vectors.
  """
  trainingClasses = getClasses(trainingSet, trainingGraph)
  trainingSamples = getSamples(trainingSet, *linkPredictors)
  classifier.fit(trainingSamples, trainingClasses)

def getSamples(nodePairs, *linkPredictors):
  """
  Returns a numpy array of shape (#nodePairs, #linkPredictors) containing the generated
  scores from the predictors for the node-pairs.
  """
  return np.column_stack(([list(zip(*p.runOnParallel(nodePairs)))[1] for p in linkPredictors]))

def getClasses(nodePairs, graph):
  """
  Returns a numpy array containing the classes (1 = link, 0 = absent link) of the given node-pairs.
  """
  return np.array(list(map(lambda p: 1 if graph.hasEdge(p[0], p[1]) else 0, nodePairs)))

def readGraph(file, percentEdges):
  # reads the konect file contained in "file" and returns two graphs: G (full
  # graph read from the file) and G1 (equal to G, but without the last nEdges
  # edges, i.e. the ones that have to be predicted)
  print("Opening file")
  f = open(file, "r")
  print("File opened")
  n = 0
  filelist = []
  #first scan to find out the number of nodes
  for line in f:
    fields = line.strip().split()
    if fields[0].startswith("%"):
      continue
    (u, v, weight, time) = [int(i) for i in fields]
    if u == v:
      continue
    filelist.append((time, u-1, v-1))
    n = max(u, v, n)
  # we sort filelist by time
  filelist.sort()
  G = Graph(n)
  # we create the graph. if an edge is created between a pair of nodes that were already connected by an edge, it is ignored and removed from the list
  filelist2 = []
  for index, (time, u, v) in enumerate(filelist):
    if not G.hasEdge(u, v):
      G.addEdge(u, v)
      filelist2.append([time, u, v])
  filelist = filelist2
  # now G is the graph with all the edges. we want to remove the last nEdges edges, creating G1
  G1 = Graph(G)
  nEdges = int(percentEdges * G.numberOfEdges())
  for (time, u, v) in filelist[-nEdges:]:
    G1.removeEdge(u, v)
  return G, G1
#Given a query location q and k, rank the users of a Geo-Social Network based
#on their distance to q, the number of their friends in the vicinity of q,
#and the connectivity of those friends and returns the top-k users.
#The focus is exploring the variations in ranking functions to achieve
#various results suited for particular applications.


#./NNTopkUsers data_GSR_Austin/queries.txt data_GSR_Austin/checkins GF1 32 100 data_GSR_Austin/socialGraph sum 1 5 1 -97.8997278214 -97.5564050674 30.1313942671 30.4098181886 390 0.485 2640

QUERY_TYPE = "GF1"

DEFAULTS = {
  "data_GSR_Austin": {
    "min-lon": -97.8997278214,
    "max-lon": -97.5564050674,
    "min-lat": 30.1313942671,
    "max-lat": 30.4098181886,
    "norm": (390,0.485,2640),
    "num-files": 1
  }
}

from util.tmp import TmpFile
from run import RUN

class GSR:
  def __init__(self, dataSet="data_GSR_Austin"):
    self.dataSet = dataSet

  def _buildQueryFile(self, q):
    self.queryFile = TmpFile()

    qf = open(self.queryFile.path, "w+")
    qf.write("{0} {1}\n".format(q[1], q[0]))
    qf.close()


  def _mapDatasetParameters(self):
    return DEFAULTS[self.dataSet]

  def _parse(self, result):
    res     = result.split("\n")
    rank    = 1
    results = [ ]
    iterator = iter(res[:-1])

    try:
      while(iterator):
        (userId, distance, score, nFriends) = iterator.next().split(",")
        friends = [ ]
        for fr in range(int(nFriends)):
          parts = iterator.next().split(",")
          fID   = int(parts[0])
          fDis  = float(parts[1])
          friends.append((fID,fDis))
        results.append({
          "userId": int(userId),
          "distance": float(distance),
          "score": float(score),
          "nFriends": int(nFriends),
          "friends": friends,
          "rank": rank,
        })
        rank = rank + 1
    except StopIteration:
      pass

    return results

  def query(self, q, r=4, k=1, fn="sum"):
    self._buildQueryFile(q)
    qParams = self._mapDatasetParameters()
    cmd = "./NNTopkUsers {0} {1}/checkins {2} {3} 100 {1}/socialGraph {4} 1 {5} {6} {7} {8} {9} {10} {11} {12} {13}".format(
      self.queryFile.path,
      self.dataSet,
      QUERY_TYPE,
      k,
      fn,
      r,
      qParams["num-files"],
      qParams["min-lon"],
      qParams["max-lon"],
      qParams["min-lat"],
      qParams["max-lat"],
      qParams["norm"][0],
      qParams["norm"][1],
      qParams["norm"][2],
    )
    return self._parse(RUN(cmd))


  def __del__(self):
    if self.queryFile:
      self.queryFile.destroy()

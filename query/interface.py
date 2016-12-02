#NSG_LAZY
# cout << "Usage: " << argv[0] << " query_file query_type k w Bpower EXTRA_FLOATING_POINT_VAR radius m" << endl;
#./NNTopkUsers query.txt NSG_LAZY 3 0.5 0 0 0 5

QUERY_TYPES = [
  "GSK_NSTP",
  "GSK_NPRU",
  "GSK_FSKR",
  "NSG_LAZY",
  "LCA_UD",
  "LCA_DD",
  "RCA",
  "HGS_UD",
  "HGS_DD",
  "GST_EAGER",
  "GST_LAZY",
]

from util.tmp import TmpFile
from run import RUN

class GSQuery:
  def __init__(self, qType="NSG_LAZY"):
    self.qType = qType

  def _buildQueryFile(self, q):
    # HARDCODED LV - FIX
    self.queryFile = TmpFile(suffix="LV")

    qf = open(self.queryFile.path, "w+")

    query = "{0} {1}".format(q['geo'][1], q['geo'][0])

    if 'keywords' in q:
      query = query + " " + ' '.join( q['keywords'] )

    qf.write(query + "\n")

    qf.close()

  def _parse(self, result):
    return {
      "status": "success",
      "output": result,
    }

  def query(self, q, k=3, w=0.5, Bpower=0, addit=0, radius=1, m=5):
    self._buildQueryFile(q)

    cmd = "./NNTopkUsers {0} {1} {2} {3} {4} {5} {6} {7}".format(
      self.queryFile.path,
      self.qType,
      k,
      w,
      Bpower,
      addit,
      radius,
      m,
    )

    return self._parse(RUN(cmd))

  def __del__(self):
    if self.queryFile:
      self.queryFile.destroy()

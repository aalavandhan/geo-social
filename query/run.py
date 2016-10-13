import shlex, os
from subprocess import call, Popen, PIPE

def RUN(cmd):
  p = Popen(shlex.split(cmd), stdin=PIPE, stdout=PIPE, stderr=PIPE, cwd=os.environ['LIB_PATH'])
  output, err = p.communicate(b"input data that is passed to subprocess' stdin")
  rc = p.returncode
  return output


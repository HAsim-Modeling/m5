import internal

from internal.stats import dump
from internal.stats import initSimStats
from internal.stats import reset
from internal.stats import StatEvent as event

def initText(filename, desc=True, compat=True):
    internal.stats.initText(filename, desc, compat)

def initMySQL(host, database, user='', passwd='', project='test', name='test',
              sample='0'):
    if not user:
        import getpass
        user = getpass.getuser()

    internal.stats.initMySQL(host, database, user, passwd, project, name,
                             sample)

#!/usr/bin/python

import os
import tempfile
import solv
import subprocess

base_url = 'http://mirror.de.leaseweb.net/epel/7/x86_64/'
repo_name = 'epel'

def download_repomd(url):
    f = tempfile.TemporaryFile()
    st = subprocess.call(['curl', '-f', '-s', '-L', url], stdout=f.fileno())
    os.lseek(f.fileno(), 0, os.SEEK_SET)

    return solv.xfopen_fd(None, f.fileno())

def download_archive(file, url):
    f = tempfile.TemporaryFile()
    st = subprocess.call(['curl', '-f', '-s', '-L', url], stdout=f.fileno())
    os.lseek(f.fileno(), 0, os.SEEK_SET)

    return solv.xfopen_fd(file, f.fileno())

def find(handle, what):
    di = handle.Dataiterator_meta(
        solv.REPOSITORY_REPOMD_TYPE,
        what,
        solv.Dataiterator.SEARCH_STRING
    )
    di.prepend_keyname(solv.REPOSITORY_REPOMD)
    for d in di:
        dp = d.parentpos()
        return dp.lookup_str(solv.REPOSITORY_REPOMD_LOCATION)
    return None

pool = solv.Pool()
pool.setarch()

handle = pool.add_repo(repo_name)
f = download_repomd(base_url + 'repodata/repomd.xml')
handle.add_repomdxml(f, 0)

primary_file_name = find(handle, 'primary')
primary_url = base_url + primary_file_name
primary_f = download_archive(primary_file_name, primary_url)
handle.add_rpmmd(primary_f, None, 0)

updateinfo_file_name = find(handle, 'updateinfo')
updateinfo_url = base_url + updateinfo_file_name
updateinfo_f = download_archive(updateinfo_file_name, updateinfo_url)
handle.add_updateinfoxml(updateinfo_f, 0)

pool.createwhatprovides()
sel = pool.Selection()
di = pool.Dataiterator(solv.SOLVABLE_NAME, 'vim', solv.Dataiterator.SEARCH_SUBSTRING|solv.Dataiterator.SEARCH_NOCASE)
for d in di:
    sel.add_raw(solv.Job.SOLVER_SOLVABLE, d.solvid)
for s in sel.solvables():
    print("%s [%s]: %s" % (s, s.repo.name, s.lookup_str(solv.SOLVABLE_SUMMARY)))


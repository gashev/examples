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

def add_ext_keys(ext, repodata, handle):
    if ext == 'DL':
        repodata.add_idarray(handle, solv.REPOSITORY_KEYS, solv.REPOSITORY_DELTAINFO)
        repodata.add_idarray(handle, solv.REPOSITORY_KEYS, solv.REPOKEY_TYPE_FLEXARRAY)
    if ext == 'FL':
        repodata.add_idarray(handle, solv.REPOSITORY_KEYS, solv.SOLVABLE_FILELIST)
        repodata.add_idarray(handle, solv.REPOSITORY_KEYS, solv.REPOKEY_TYPE_DIRSTRARRAY)
   
def add_ext(handle, repodata, what, ext):
    filename = find(handle, what)
    if not filename and what == 'deltainfo':
        filename = find(handle, 'prestodelta')
    if not filename:
        return

    new_handle = repodata.new_handle()
    repodata.set_poolstr(new_handle, solv.REPOSITORY_REPOMD_TYPE, what)
    repodata.set_str(new_handle, solv.REPOSITORY_REPOMD_LOCATION, filename)
    add_ext_keys(ext, repodata, new_handle)
    repodata.add_flexarray(solv.SOLVID_META, solv.REPOSITORY_EXTERNAL, new_handle)
 
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

repodata = handle.add_repodata(0)
add_ext(handle, repodata, 'deltainfo', 'DL')
add_ext(handle, repodata, 'filelists', 'FL')
repodata.internalize()

handle.create_stubs()
pool.createwhatprovides()

f_solv = open('/tmp/solv' , 'wb')
f_solv = solv.xfopen_fd(None, f_solv.fileno())
handle.write(f_solv)
f_solv.close()

import urllib2
import urlparse
import base64
import os
import json
import installers
import unpackers
import logging
from buildenv import buildenv


def findpath(dirname, obj, search='children'):
    if search == 'parents':
        while dirname != '/':
            if os.path.exists(os.path.join(dirname, obj)):
                return dirname
            else:
                dirname = os.path.dirname(dirname)
    else:
        pass

def gitwsfetch(url, **kwargs):
    workspace = kwargs.get("workspace", "{}-{}".format(kwargs.get("name"), kwargs.get("version")))
    cache_dir = kwargs.pop("directory", ".cache")
    workspace = os.path.join(cache_dir, kwargs.get("workspace", workspace ))
    if os.path.exists(workspace):
        return workspace
    rootdir = findpath(os.getcwd(), ".git", search='parents')
    relpath  = kwargs.get('path')
    if relpath is not None:
        rootdir = os.path.join(rootdir, relpath)
    if kwargs.get('copymode', 'link') == 'link':
        os.symlink(rootdir, workspace)
    return workspace


def urlfetch(url, **kwargs):
    if kwargs.get('file'):
        filename = kwargs.get("file")
        directory = kwargs.pop('directory', '.')
        filename = os.path.abspath(os.path.join(directory, filename))
        if os.path.exists(filename):
            return filename

    request = urllib2.Request(url)
    auth = kwargs.get('auth')
    if auth:
        request.add_header("Authorization", "Basic {}".format(base64.encodestring(":".join(auth))).strip())
    proxy_handler = urllib2.ProxyHandler({"https": "proxy-chain.intel.com:911",
                                          "http": "proxy-chain.intel.com:911"})
    urllib2.install_opener(urllib2.build_opener(proxy_handler))
    fd = urllib2.urlopen(request, timeout=2)
    if kwargs.get('file'):
        dirname = os.path.dirname(filename)
        if not os.path.exists(dirname):
            os.makedirs(dirname)
        with open(filename, "w") as fd2:
            fd2.write(fd.read())
        return filename
    elif kwargs.get('json'):
        text = fd.read()
        return json.loads(text)
    else:
        return fd.read()

def artifactory(url, **kwargs):
    files = kwargs.pop("files", None)
    if files is None:
        return urlfetch(url, **kwargs)
    baseurl = kwargs.pop("base")
    repo = kwargs.pop("repo")
    fetched = []
    kwargs.pop("directory")
    for rpath,lpath in files:
        if rpath.endswith("/*") or rpath == "*":
            dirname = os.path.dirname(rpath)
            if dirname:
                url2 = urlparse.urljoin(url, dirnddame)
                lpath = os.path.join(lpath, dirname)
            else:
                url2 = url
            endpath = url.index(repo)
            endpath = url[endpath:]
            uriinfo = urlfetch(urlparse.urljoin(baseurl, "artifactory/api/storage/{}".format(endpath)), json=True, **kwargs)
            for child in uriinfo.get("children",[]):
                uri = child["uri"].strip("/")
                url3 = urlparse.urljoin(baseurl, "artifactory/{}/{}".format(endpath, uri))
                if child["folder"]:
                    fetched.extend(artifactory(url3, base=baseurl, files=[("*", os.path.join(lpath, uri))]))
                else:
                    downloaded = urlfetch(url3, file=os.path.join(lpath, uri), **kwargs)
                    fetched.append(downloaded)
        else:
            basename = os.path.basename(rpath)
            fetched.append(urlfetch(urlparse.urljoin(url, rpath), file=os.path.join(lpath, basename), **kwargs))
    return fetched


def gitfetch(url, **kwargs):
    directory = os.path.abspath(kwargs.pop('directory', '.'))
    workspace = os.path.join(directory, kwargs.get("workspace"))
    if os.path.exists(workspace):
        return workspace
    installers.run("git clone {} {}".format(url, workspace), env=kwargs.get("env"))
    return workspace

FETCHERS = { "urlfetcher" : urlfetch,
             "gitfetcher" : gitfetch,
             "gitwsfetch" : gitwsfetch,
             "artifactory" : artifactory}

def get(fetcher_name):
    return FETCHERS.get(fetcher_name)

def get_package(package, packages_dir=".packages", cache_dir=".cache"):
    cwd = os.getcwd()
    packages_dir = os.path.abspath(packages_dir)
    exit_code = 0
    url = package['url']
    fetcher_info = package['fetcher']
    fetcher_name = fetcher_info.pop('name')
    fetcher = FETCHERS.get(fetcher_name)
    logging.info("fetching {}".format(url))
    checkout = fetcher(url, directory=cache_dir, **fetcher_info)
    if "unpacker" in package:
        unpacker = unpackers.get(package["unpacker"])
        logging.info("unpacking {}".format(checkout))
        checkout = unpacker.unpack(checkout, cache_dir)
    installer_info = package.get("installer")
    if installer_info:
        installer_name = installer_info.pop("name")
        installer = installers.get(installer_name)
        install_dir = os.path.abspath(package.get("install", packages_dir))
        logging.info("{} installing {}".format(installer_name, package["name"]))
        return installer.install(checkout, install_dir, env=buildenv(cwd), **installer_info)
    return exit_code
    

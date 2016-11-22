import os
import tarfile
import tempfile


class tarunpack(object):
    def unpack(self, filename, destination):
        base, ext = os.path.splitext(os.path.basename(filename))
        if ext == '.gz' or ext == 'tar.gz' or ext == 'tgz':
            mode = 'r:gz'
        elif ext == '.bzip2':
            mode = 'r:bz2'
        tf = tarfile.open(filename, mode)
        rootdir = os.path.commonprefix(tf.getnames())
        unpackdir = destination if '' != rootdir else os.path.join(destination, base)
        tf.extractall(path=unpackdir)
        unpackdir = os.path.join(unpackdir, rootdir) if rootdir != '' else unpackdir
        return unpackdir

UNPACKERS = { "tar": tarunpack}

def get(name):
    return UNPACKERS.get(name)()

#!/usr/bin/env python
import argparse
import subprocess
import json
from os import path

USERS = {
    'enno'    : 'enno.luebbers@intel.com',
    'henry'   : 'henry.mitchel@intel.com',
    'joe'     : 'joe.grecco@intel.com',
    'ru'      : 'ru.pan@intel.com',
    'rodrigo' : 'rodrigo.rojo@intel.com',
    'rahul'   : 'rahul.r.sharma@intel.com',
    'ananda'  : 'ananda.ravuri@intel.com',
    'sadruta' : 'sadruta.chandrashekar@intel.com',
    'abelardo': 'abelardo.jara-berrocal@intel.com',
    'bob'     : 'robertx.lacasse@intel.com',
    'tim'     : 'tim.whisonant@intel.com',
    'g'       : 'enno,henry' }

def fromgroups(groups):
    for group in groups:
        for user in USERS.get(group, '').split(','):
            u = USERS.get(user):
            if u:
                yield u


if __name__ == '__main__':
    # use json file ~/.gitpush.json for additional mappings
    # format of file has to be like:
    # {
    #    "users" :
    #             {
    #               "name1" : "email1",
    #               "name2" : "email2"
    #             }
    # }
    if path.exists(path.expanduser("~/.gitpush.json")):
        try:
            with open(path.expanduser("~/.gitpush.json")) as fd:
                config = json.load(fd)
                USERS.update(config["users"])
        except:
            import traceback
            print "error parsing ~/.gitpush.json"
            traceback.print_exc()
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--cc', nargs='*', default=[],
                        help='list of users to add to cc list')
    parser.add_argument('-r', '--reviewers', nargs='*', default=[],
                        help='list of users to add to reviewers list')
    parser.add_argument('-b', '--branch', default='develop',
                        help='name of branch to push for review')
    parser.add_argument('-g', '--groups', nargs='*', default=[],
                        help='group names to get users from')
    parser.add_argument('-R', '--remote', default='origin',
                        help='name or url of remote repository to push to')
    parser.add_argument('-p', '--pretend', action='store_true', default=False,
                        help='Do not run the actual command, simply print it out')

    args = parser.parse_args()
    cmd = 'git push {} HEAD:refs/for/{}'.format(args.remote, args.branch)
    if args.cc or args.reviewers or args.groups or USERS.get('default',[]):
        cmd += '%{}'.format(','.join(['r={}'.format(USERS[k])  for k in args.reviewers] +
                                     ['cc={}'.format(USERS[k]) for k in args.cc] +
                                     ['r={}'.format(u) for u in fromgroups(args.groups + ['default'])]))
    print(cmd)
    if not args.pretend:
        subprocess.call(cmd, shell=True)

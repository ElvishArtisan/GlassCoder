#!%PYTHON_BANGPATH%

# pypad_glasscoder.py
#
# Send articulated PAD updates to an instance of glasscoder(1).
#
#   (C) Copyright 2019 Fred Gleason <fredg@paravelsystems.com>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2 as
#   published by the Free Software Foundation.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public
#   License along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

import sys
import syslog
import configparser
import pycurl
import pypad

def eprint(*args,**kwargs):
    print(*args,file=sys.stderr,**kwargs)

def ProcessPad(update):
    if update.config().has_section('Glasscoder'):
        update_url=update.config().get('Glasscoder','UpdateUrl')
        n=1
        lines=[]
        section='Line'+str(n)
        while(update.config().has_section(section)):
            lines.append('"'+update.config().get(section,'Key')+'": "'+update.resolvePadFields(update.config().get(section,'Value'),pypad.ESCAPE_JSON)+'"')
            n=n+1
            section='Line'+str(n)

        if update.shouldBeProcessed('Glasscoder'):
            json='{\r\n'
            json+='    "Metadata": {\r\n'
            for line in lines:
                json+='        '+line+',\r\n'
            json=json[0:-3]+'\r\n'
            json+='    }\r\n'
            json+='}\r\n'

            curl=pycurl.Curl()
            curl.setopt(curl.URL,update_url+'/json_pad');
            curl.setopt(curl.POST,True)
            curl.setopt(curl.POSTFIELDS,json)
            try:
                curl.perform()
            except pycurl.error:
                update.syslog(syslog.LOG_WARNING,'update failed: '+curl.errstr())
            curl.close()


#
# 'Main' function
#
rcvr=pypad.Receiver()
try:
    rcvr.setConfigFile(sys.argv[3])
except IndexError:
    eprint('pypad_glasscoder.py: USAGE: cmd <hostname> <port> <config>')
    sys.exit(1)
rcvr.setPadCallback(ProcessPad)
rcvr.start(sys.argv[1],int(sys.argv[2]))

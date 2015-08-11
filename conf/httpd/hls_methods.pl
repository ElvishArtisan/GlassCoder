#!/usr/bin/perl

# hls_methods.pl
#
# HTTP methods script for HLS streaming
#
# (C) 2015 Fred Gleason <fredg@paravelsystems.com>
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

sub Put
{
    my $response=201;
    my $file_name=&DestinationPath();
    my $file_len = $ENV{'CONTENT_LENGTH'};

    if(stat($file_name)) {
	$response=204;
    }

    #
    # Copy Data
    #
    my $toread = $file_len;
    $content = "";
    while ($toread > 0)
    {
	$nread = read(STDIN, $data, $file_len);
	$toread -= $nread;
	$content = $data;
    }
    if(!open(OUT, "> $file_name")) {
	&Reply(500,"PUT failed!\n");
    }
    print OUT $content;
    close(OUT);

    &Reply($response,"OK");
}


sub Delete
{
    my $file_name=&DestinationPath();
    if(stat($file_name)) {
	if(!unlink($file_name)) {
	    &Reply(500,"Unable to delete\n");
	}
    }
    &Reply(204,"OK");
}


sub DestinationPath()
{
    my $file_name=$ENV{'PATH_TRANSLATED'};
    if(!$file_name) {
	&Reply(500,"No destination path provided.");
    }
    return $file_name;
}

sub Reply
{
    print "Status: ".$_[0]."\n";
    print "Content-type: text/html\n";
    print "\n";
    print $_[1];
    exit 0;
}


#
# Verify Method
#
if($ENV{'REQUEST_METHOD'} eq "PUT") {
    &Put();
}
if($ENV{'REQUEST_METHOD'} eq "DELETE") {
    &Delete();
}
&Reply(500,"Unsupported method.\n");




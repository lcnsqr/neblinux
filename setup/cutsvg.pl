#!/usr/bin/perl

#binmode(STDOUT, ":utf8");

use IO::Handle;

my $prev = '';

open($plot, '>', $ARGV[0]) or die $!;

while (<STDIN>){
  if ( $_ =~ /^<\?xml/ ) {
    truncate $plot, 0;
    seek($plot, 0, 0);
  }

  $prev = $_;
  print $plot $prev;
	if ( $_ =~ /^<\/svg>$/ ) {
		$plot->flush();
  }
}

close($plot);

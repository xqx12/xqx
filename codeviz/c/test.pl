#!/usr/bin/perl


$count = 0;
$index = 0;

$file = "input.txt";

open (FH, "+< $file")               or die "can't update $file: $!";

dellastline(FH);

close(FH);

sub dellastline($) {
  my $FH = @_;
  while ( <FH> ) {
	$addr = tell(FH) unless eof(FH); 
  }
  print "line=$addr FH=FH\n";
  truncate(FH, $addr); 

}

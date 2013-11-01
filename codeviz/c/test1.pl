#!/usr/bin/perl


$count = 0;
$index = 0;

open (IN, "input.txt");
open (TEMP, ">temp.txt");
while(<IN>){
	print TEMP $_;
	$count++;
}
close(TEMP);
close(IN);

open (TEMP, "temp.txt");
open (IN, ">input.txt");
while(<TEMP>){
	if($index > 0 && $index < $count-1){
		print IN $_;
	}
	$index++;
}
close(IN);
close(TEMP); 

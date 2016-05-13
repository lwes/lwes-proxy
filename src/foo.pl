#!/usr/bin/env perl

use strict;
use warnings;

my %counts_by_milli;
my %bytes_by_milli;

while (my $line = <ARGV>) {
  chomp $line;
  my ($sec, $mill, $bytes) = split /\t/, $line;
  $counts_by_milli{$mill} += 1;
  $bytes_by_milli{$mill} += $bytes;
}

foreach my $key (sort keys %counts_by_milli) {
  print $key."\t".$counts_by_milli{$key}."\t".$bytes_by_milli{$key}."\n";
}

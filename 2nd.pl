#!/usr/bin/perl

#
# 2008.04.18 구현 되었던 범위까지 구현
# 사용법
# #>perl 1st.pl test_log
#

$FILENAME="./test_log";
$KTF_PARAM="KTF";
$ALMNO_PARAM="S2193";
$ITEM_PARAM="INTERACTIVE";
$CATEGORY_PARAM="ORIG";


# argument test
print "$#ARGV\n";
@KTF=shift(@ARGV);
print "$#ARGV :: @KTF\n";
@KTF2=shift(@ARGV);
print "$#ARGV :: @KTF2\n";

sub read_mem
{
        @files = glob("./test_log");
		$flag = 1;
        foreach $file (sort @files) {
			open STG, "$file" || die $!;

			#print "file name: $file\n";

			while($line = <STG>) {

				chop($line);
				if($line =~ /$KTF_PARAM/) {
				@line=split(/ /,$line);
				$TIME=$line[12];

				$line = <STG>;
				if($line =~ /$ALMNO_PARAM/){

					$line = <STG>;
					if($line =~ /STM/){
						@line=split(/=/,$line);
						if( $line[2] == '5(MN)' ){
							$line = <STG>;
							if($line =~ /$ITEM_PARAM/){
								$line = <STG>;
								if($line =~/$CATEGORY_PARAM/){
									$line=<STG>;
									$line=<STG>;
									if($line =~/RNC/){

										# TITLE ######################################

										if( $flag > 0 ){
											@line=split(/\s+/,$line);
											$line=<STG>;
											@line2=split(/\s+/,$line);
											$line=<STG>;
											@line3=split(/\s+/,$line);
											print "TIME     @line@line2@line3\n";
											$flag = 0;
										}
										else{
											$line=<STG>;$line=<STG>;
										}

										until($line=~ /COMPLETE/){

											# DATA ##################################

											$line=<STG>;
											@line=split(/\s+/,$line); #print "#$#line# @line\n";
											$line=<STG>;
											@line2=split(/\s+/,$line); #print "#$#line2# @line2\n";
											$line=<STG>;
											@line3=split(/\s+/,$line); #print "#$#line3# @line3\n";
											@total=(@line,@line2,@line3);
											#print "######## $#total:@line[1] # @total\n";

printf("%-5s  %-5s  %-5s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s  %-6s\n",
$TIME, 
$total[1], $total[2], $total[3], $total[4], $total[5], $total[6], $total[7], $total[8], 
$total[9], $total[10], $total[12], $total[13], $total[14], $total[15], $total[16], $total[17], 
$total[18], $total[19], $total[20], $total[22], $total[23], $total[24], $total[25], $total[26], 
$total[27], $total[28], $total[29]);

											if( @line[1] >= 327 ){ last; }
										
										}

									}
								}
							}
						}
					}
				}
				
				
			}
			
			

#                        if($line =~ /CPU_COMPUTE/) {
#				print "test2: $line\n";
#                                foreach $key (keys %data) {
#                                        $data{$key} = 0;
#                                }
#                        }

#                        if($line =~ /DEL_NODE=([^\r\n \t]+)/) {
##                               print "SIP=$1 DIP=$3\n";
#                                $tmp = "$1";
#                                $data{$tmp}++;
#                        }
                }
				print "fine.\n";
        }

        foreach $key (keys %data) {
                print "$key\t$data{$key}\n";
        }
}

read_mem;
exit
####[04/15 03:12:28.199049][INF:18434] [CPU_COMPUTE  MPCPU = [  1.10] TOT/IDL[1000][989]  OLDSTAT = [3]]

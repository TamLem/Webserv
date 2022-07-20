#!/usr/bin/perl
#
# A simple perl CGI sudoku solver.
#
# Place it in a folder where cgi can see it and you're good to go.
#
# Written by Pedro
#
# For more stupid scripts like this one check out http://lamehacks.net
#

use CGI;
$cgi = new CGI;

$debug = 0;

if(!$cgi->param('puzzle') && !$cgi->param('solve')) {show_html_form(); die();}

#everything is displayed in plain text format, except the input form  which we are not going to print if we didn't so far
print "Content-type: text/plain\r\n\r\n";

if(!$cgi->param('puzzle')){
  $puzzleraw = process_input_from_html_form($cgi);
}
else{
  $puzzleraw = $cgi->param('puzzle');
}

#So we can use both dots and zeros
$puzzleraw =~ s/\./0/g;

#'pipes' can be used to improve human reading of puzzles as they'll be ignored
$puzzleraw =~ s/\|//g;

#trim the puzzle
$puzzleraw =~ s/^\s+//;
$puzzleraw =~ s/\s+$//;

             
#validate the puzzle
if ($puzzleraw !~ /^\d{81}$/) { print "Error! Invalid puzzle."; die(); }

#put it into a usable format
@puzzle = &put_into_array($puzzleraw);

if (!check_puzzle(@puzzle)){ print "Error! initial puzzle data breaks sudoku rules."; die(); };

#solve it!
solve(@puzzle);

#++++++++++++++++++++++++++++ S U B R O U T I N E S ++++++++++++++++++++++++++++

#===============================================================================
# The main solving function - where everything comes together
#===============================================================================
sub solve{
	my @puzzleC = @_;	

	if (!have_empty_cells(@puzzleC)){
		print "Hurray! Puzzle solved!\n";
		&print_puzzle_array(@puzzleC);
		die("yeeepee!\n");
		}
		
	my $empty_cell_position = find_first_empty_cell(@puzzleC);
	$puzzleC[0] = $empty_cell_position;
	
	my @cell_possibilities = (); 
	@cell_possibilities = get_possibilities(@puzzleC);
	if ($#cell_possibilities == -1){ return -1; } 
	for my $possible_value (@cell_possibilities){
		$puzzleC[$empty_cell_position] = $possible_value;
		solve(@puzzleC);
	}
}

#===============================================================================
#	uniquify an array
#===============================================================================
sub uniquify{
	my %return_these_keys;
	for (@_){
		$return_these_keys{$_}=1;
	}
	return keys %return_these_keys;
}

#===============================================================================
#	Put the puzzle in a friendly array
#===============================================================================
sub put_into_array{
	@puzzlearrayraw = split(//, $_[0]);

	$i=11;
	foreach(@puzzlearrayraw){
		if($i%10 == 0){$i++;}
		$puzzleboarded[$i]=$_;
		$i++;
	}
	return @puzzleboarded;
}

#===============================================================================
#	Print the friendly array puzzle in a human readable format
#===============================================================================
sub print_puzzle_array{
	print "\n+-----------+\n";
	for $i (1..9){	
		print "|";
		for $j (1..9){	
			print $_[$i*10+$j];
			print "|" if !($j%3);
		}
		print "\n";
		if (!($i%3) and $i<9){print "|---+---+---|\n";}
	}
	print "+-----------+\n";
}


#===============================================================================
#	Check if the initial puzzle breaks any sudou rule
#	$_[0]  Contains the puzzle in array format
#===============================================================================
sub check_puzzle{
	for (my $i = 1; $i < 10; $i++) {
		for (my $j = 1; $j < 10; $j++) {
			my $cell_index = $i*10+$j;
			if(defined($_[$cell_index]) && $_[$cell_index] != 0){
				if (!check_collisions($cell_index, $_[$cell_index], \@_)) {return 0;}
			}
		}	
	}
	return 1; #true
}

#===============================================================================
#	Check if a cell breaks any sudoku rule
#	$_[0]  cell position
#	$_[1]  cell value
#	@{$_[2]}  puzzle in array format <---- pass a reference, i.e. prepend a "\" to the array 
#===============================================================================
sub check_collisions{	
	my $cell_position = $_[0];
	my $cell_value = $_[1]; #unnecessary, remove and change call acordingly
	my @mypuzzle = @{$_[2]};
  
	my @neighbours = &get_neighbours($cell_position);
   	
	for my $i (@neighbours){
	  if ($mypuzzle[$i] != 0){
      if($cell_value == $mypuzzle[$i]){return 0; }
    }			
	}
	return 1;
}

#===============================================================================
#	Check if a puzzle has empty cells
#===============================================================================
sub have_empty_cells{
	foreach (@_){
		if(defined($_) && $_==0){ return true; }
	}
	return 0;
}


#===============================================================================
#	find the first empty cell
#===============================================================================
sub find_first_empty_cell{	
	my @puzzle = @_;
	
	for ($i=11;$i<100;$i++){
		if ($i%10!=0 && $_[$i]==0){
			return $i;
		}
	}
	return 0; #puzzle without empty cells
}

#===============================================================================
#	get all 'neighbours' positions
#===============================================================================
sub get_neighbours{
	my $cell_position = $_[0];
	my $cell_column = $cell_position % 10;
	my $cell_row = ($cell_position - $cell_column)/10;	
  $subsquare_neighbourhood[1]=[11,12,13,21,22,23,31,32,33];
  $subsquare_neighbourhood[2]=[14,15,16,24,25,26,34,35,36];	
  $subsquare_neighbourhood[3]=[17,18,19,27,28,29,37,38,39];
  $subsquare_neighbourhood[4]=[41,42,43,51,52,53,61,62,63];
  $subsquare_neighbourhood[5]=[44,45,46,54,55,56,64,65,66];
  $subsquare_neighbourhood[6]=[47,48,49,57,58,59,67,68,69];
  $subsquare_neighbourhood[7]=[71,72,73,81,82,83,91,92,93];
  $subsquare_neighbourhood[8]=[74,75,76,84,85,86,94,95,96];
  $subsquare_neighbourhood[9]=[77,78,79,87,88,89,97,98,99];
  my $subsquare;
		
	$foundit=0;
	for (my $ii = 1; $ii < 10; $ii++) {
		for (my $jj = 0; $jj < 9; $jj++){
			if ($cell_position == $subsquare_neighbourhood[$ii][$jj]){
				$subsquare = $ii;
				$foundit = 1;
				break;
			}
		}
		if ($foundit==1){break;}		
	}
	
	$reff = $subsquare_neighbourhood[$subsquare];
	@all = @$reff;
	
	#columns and row neighbours
	for (my $i = 1; $i < 10; $i++) {		
		push @all, (10*$cell_row + $i , 10*$i + $cell_column);
	}

	#remove own cell
	@neighbours = ();
	foreach $var (@all){
		if ($var != $cell_position) {
			push @neighbours, $var;
		}
	}
	
	@neighbours = &uniquify(@neighbours);	
	return @neighbours;
}

#===============================================================================
#	get possibilities for a given cell
#===============================================================================
sub get_possibilities{
	my @puzzle = @_;
	my $cell_position = $_[0];
	my @used_values;
	my @possibilities;
	
	my @neighbours = &get_neighbours($cell_position);
	
	for (@neighbours) {
		if ($puzzle[$_] != 0){$used_values[$puzzle[$_]]=1;}	
	}
	
	for (1..9){
		unless ($used_values[$_]==1) {push @possibilities, $_;}	
	}
	
	return @possibilities;
}

#===============================================================================
#	builds a puzzleraw string from html table form input
#===============================================================================
sub process_input_from_html_form{
  my $mycgi = $_[0];
  my $mypuzzleraw = "";
  for (my $i = 1; $i <= 9; $i++) {
		for (my $j = 1; $j <= 9; $j++){
		    if($mycgi->param("c$i$j") !~ /^\d{0,1}$/){
          print "input does not validate in this cell!!! $i $j \n";
          die;
        }
        else{
          if(length $mycgi->param("c$i$j") == 0){ $mypuzzleraw = $mypuzzleraw . "0";}
          else {$mypuzzleraw = $mypuzzleraw . $mycgi->param("c$i$j");}
        }
    }
  }
  return $mypuzzleraw;  
}

#===============================================================================
#	output html page with form
#===============================================================================
sub show_html_form{
	$html = <<'HTMLDATA';
<html>
    <head>
        <title>sudoku solver</title>
    </head>
    <body>
<h1>lamehacks.net sudoku solver</h1>
<a href="/">Back to lamehacks.com</a>
<p>
    Fill in your sudoku puzzle and click 'Solve me!'
</p>
<form action="sudoku.pl" >
    <table border="1">
        <tbody>
            <tr>
                <td>
                    <table border="1">
                        <tbody>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c11" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c12" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c13" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c21" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c22" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c23" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c31" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c32" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c33" />
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </td>
                <td>
                    <table border="1">
                        <tbody>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c14" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c15" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c16" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c24" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c25" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c26" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c34" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c35" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c36" />
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </td>
                <td>
                    <table border="1">
                        <tbody>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c17" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c18" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c19" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c27" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c28" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c29" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c37" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c38" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c39" />
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </td>
            </tr>
            <tr>
                <td>
                    <table border="1">
                        <tbody>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c41" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c42" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c43" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c51" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c52" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c53" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c61" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c62" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c63" />
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </td>
                <td>
                    <table border="1">
                        <tbody>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c44" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c45" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c46" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c54" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c55" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c56" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c64" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c65" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c66" />
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </td>
                <td>
                    <table border="1">
                        <tbody>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c47" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c48" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c49" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c57" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c58" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c59" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c67" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c68" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c69" />
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </td>
            </tr>
            <tr>
                <td>
                    <table border="1">
                        <tbody>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c71" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c72" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c73" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c81" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c82" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c83" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c91" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c92" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c93" />
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </td>
                <td>
                    <table border="1">
                        <tbody>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c74" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c75" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c76" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c84" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c85" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c86" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c94" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c95" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c96" />
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </td>
                <td>
                    <table border="1">
                        <tbody>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c77" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c78" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c79" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c87" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c88" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c89" />
                                </td>
                            </tr>
                            <tr>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c97" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c98" />
                                </td>
                                <td>
                                    <input type="text" maxlength="1" size="1" name="c99" />
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </td>
            </tr>
        </tbody>
    </table>
    <input type="submit" value="Solve me!" name="solve" />
    <br />
</form>
<p>If you preffer you can insert the puzzle in a raw format:</p>
<form action="sudoku.pl">
    <input type="text" name="puzzle" value="" size="75" /><br />
    <input type="submit" value="Solve me!" name="solveraw" />
</form>
<p>Use zeros or dots for blanks. You can use vertical bars for visual aid as they will be ignored.</p>
<p><strong>Examples:</strong><br/><br/>
.14.6.3..62...4..9.8..5.6...6.2....3.7..1..5.5....9.6...6.2..3.1..5...92..7.9.41.
<br /><br />
|.14.6.3..|62...4..9|.8..5.6..|.6.2....3|.7..1..5.|5....9.6.|..6.2..3.|1..5...92|..7.9.41.|
<br /><br />
|014060300|620004009|080050600|060200003|070010050|500009060|006020030|100500092|007090410|
<br /><br />
014060300620004009080050600060200003070010050500009060006020030100500092007090410
</p>
    </body>
</html>
HTMLDATA
  print "Content-type: text/html\r\n\r\n";
	print $html;
}
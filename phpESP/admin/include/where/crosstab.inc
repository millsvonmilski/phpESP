<?php

 # $Id$

/* The Cross Tabulation feature has been kept seperate from the rest of phpESP intentionally.
   It provides an easier means of disabling it by commenting out the require line in results.inc
   and the function call (cross_tabulation()) plus it avoids cluttering survey_results...
*/
if (!defined('ESP_BASE'))
                define('ESP_BASE', dirname(__FILE__) . '/../');

        require_once(ESP_BASE . '/admin/include/lib/esphtml.forms.inc');

/* {{{ proto void cross_tabulation (integer sid, integer qidc, integer qidr, integer qid, array cids)
  Builds HTML to allow for cross tabulation reporting. */
  function cross_tabulation($sid, $qidc, $qidr, $qid, $cids){
  // some error handling
        if (empty($qid) && empty($cids)){
                if (!empty($qidc) && !empty($qidr)){
                        if ($qidc == $qidr){
                                $errmsg = mkerror(_('Error cross-tabulating. Please ensure that your column
                                             and row selections are not of the same question.'));
                        }
                        else {
                                process($sid,$qidc,$qidr);
                        }
                }
                else if (!empty($qidc) || !empty($qidr)){
                        $errmsg = mkerror(_('Error cross-tabulating. Please ensure you make both
                                     a column and row selection.'));
                }
        }
        else {
                $errmsg = mkerror(_('Error cross-tabulating. You are attempting to cross analyse
                             and tabulate at the same time. This is not possible!'));
        }
        if (!empty($errmsg)){
                echo($errmsg);
        }

  }
  function process($sid, $qidc, $qidr){
        $qids = array();
	$cids_col = array();
        $cids_row = array();
        $qidc_content = array();
        $qidr_content = array();
        $row_totals = array();
        $col_counts = array();
        $qn_content = array();
        $tbl = array();
        $tids = array();
	$tid_content = array('Y','N');
        $count = 0;
	$tid_count = 0;
	$p = 0;
    
	 
        array_push($qids, $qidc);
        array_push($qids, $qidr);

	foreach ($qids as $qid){
		$sql = "SELECT content FROM question
			WHERE survey_id='${sid}'
			AND id = '${qid}'";
		$tid_result = mysql_query($sql);
		$result = mysql_fetch_row($tid_result);
		
		array_push($qn_content, $result[0]);
	}
	mysql_free_result($tid_result);
        unset($sql);
        unset($result);
	
	// let's determine each question's type.
	foreach ($qids as $qid){
		$sql = "SELECT type_id FROM question
			WHERE survey_id='${sid}'
			AND id = '${qid}'";
		$tid_result = mysql_query($sql);
		$result = mysql_fetch_row($tid_result);
		
		array_push($tids, $result[0]);
	}
	mysql_free_result($tid_result);
        unset($sql);
        unset($result);
	// Now that we know the question's type	
	// let's grab each question's choice content.
	foreach ($tids as $tid){
		if ($tid_count == 0 && $tid == 1){
			foreach($tid_content as $content){
				if ($content == 'Y'){
					$content = 'Yes';
				}
				else {
					$content = 'No';
				}
				array_push($qidc_content, $content);
			}
		}
		else if ($tid_count == 0){
			$sql = "SELECT content FROM question_choice
				WHERE question_id='${qidc}'
				ORDER BY id";
			if ($result = mysql_query($sql)){
				while (list($row) = mysql_fetch_row($result)){
					array_push($qidc_content, $row);
				}
				mysql_free_result($result);
				unset($sql);
			}
		}
		else if ($tid == 1){
			foreach($tid_content as $content){
				if ($content == 'Y'){
					$content = 'Yes';
				}
				else {
					$content = 'No';
				}
				array_push($qidr_content, $content);
			}
		}
		else{
			$sql = "SELECT content FROM question_choice
                                WHERE question_id='${qidr}'
                                ORDER BY id";
			if ($result = mysql_query($sql)){
				while (list($row) = mysql_fetch_row($result)){
                	                array_push($qidr_content, $row);
                                }
                                mysql_free_result($result);
                                unset($sql);	
			}
		}
		$tid_count++;
	}
	reset($tids);
	$tidcounter = 0;
	// Now to build the tabulation arrays that hold response counts.
	foreach ($qids as $qid){
		switch ($tids[$tidcounter]){
			case 1:
				$from_table = 'response_bool';
				$order = 'id';
				$table = 'response_bool';
				array_push($tbl, $table);
				break;
			case 4:
			case 6:
				$from_table = 'question_choice';
				$order = 'id';
				$table = 'response_single';
				array_push($tbl, $table);
				break;
			case 5:
				$from_table = 'question_choice';
				$order = 'id';
				$table = 'response_multiple';
				array_push($tbl, $table);
				break;
		}
		if ($tids[$tidcounter] == 1){
			if ($tidcounter == 0){
				foreach($tid_content as $content){
					array_push($cids_col, $content);
				}
			}
			else{
				foreach($tid_content as $content){
                                        array_push($cids_row, $content);
				}
			}
		}
		else {
			$sql = "SELECT id FROM $from_table
				WHERE question_id = $qids[$count]
				ORDER BY $order";
			$result = mysql_query($sql);
			while ($cid = mysql_fetch_row($result)){
				if ($count == 0){
					array_push($cids_col, $cid[0]);
				}
				else{
					array_push($cids_row, $cid[0]);
				}
			}
	mysql_free_result($result);
		}
		$count++;
		$tidcounter++;	
	}

	$num = 0;
	$cids_col_count = sizeof($cids_col);
	$cids_row_count = sizeof($cids_row);

        foreach ($cids_row as $yy){
        	foreach ($cids_col as $xx){
                	$sql1 = "SELECT count(*) FROM $tbl[0] r1, $tbl[1] r2
                                 WHERE (r1.question_id = '${qids[0]}' AND r1.choice_id = '${xx}')
                                 AND (r2.question_id = '${qids[1]}' AND r2.choice_id = '${yy}')
                                 AND r1.response_id = r2.response_id";
                        $result1 = mysql_query($sql1);
                        while ($counts = mysql_fetch_row($result1)){
                        	$total_row += intval($counts[0]);
                                array_push($col_counts, $counts[0]);
                        }
		}
                array_push($row_totals,$total_row);
                $total_row = 0;
	}                

                // let's build the cross-tabulation results table
?>
                <center>
		<p><b><?php echo("\"<font color=\"#CC0000\">".$qn_content[1]."</font>\""); ?>
		      <?php echo ("vs \"<font color=\"#0099FF\">".$qn_content[0]."</font>\""); ?></b></p>
                <table width="714" border="0" cellpadding="5" cellspacing="1" bgcolor="#999999">
                <tr>
                <td width="411" height="25" valign="default" bgcolor="#FFFFFF"><!-- <div align="center">
		<strong>graph cross tabulated results:&nbsp;&nbsp;&nbsp;&nbsp; 
                </strong></div> --></td>
<?php
                foreach ($qidc_content as $content){
                        //echo ("<td width=\"89\" bgcolor=\"#FFFFFF\">");
                        echo ("<td bgcolor=\"#FFFFFF\">");
                        echo ("<div align=\"center\"><font color=\"#0099FF\"><b>".$content."</b></font></div></td>\n");
                }
                unset($content);
?>
                <td bgcolor="#CCCCCC">
                <div align="center"><strong>Totals</strong></div></td>
                </tr>
<?php
                $q = 0;
                $counter = 0;
                $rt = 0;
                foreach ($col_counts as $tt){
                        $counter++;
                        if ($counter == 1){
                                echo ("<tr>\n");
                                echo ("<td height=\"30\" valign=\"default\" bgcolor=\"#FFFFFF\">");
                                echo ("<font color=\"#CC0000\">".$qidr_content[$q++].":</font></td>\n");
                        }
                        if ($counter == $cids_col_count+1){
                                echo ("<td valign=\"default\" bgcolor=\"#CCCCCC\">");
                                echo ("<div align=\"center\"><b>".$row_totals[$rt++]."</b></div></td>\n");
                                echo ("</tr>\n");
                                echo ("<tr><td height=\"30\" valign=\"default\" bgcolor=\"#FFFFFF\">");
                                echo ("<font color=\"#CC0000\">".$qidr_content[$q++].":</font></td>\n");
                                echo ("<td valign=\"default\" bgcolor=\"#FFFFFF\">");
                                echo ("<div align=\"center\">".$tt."</div></td>\n");
                                $counter = 1;
                        }
                        else {
                                echo ("<td valign=\"default\" bgcolor=\"#FFFFFF\">");
                                echo ("<div align=\"center\">".$tt."</div></td>\n");
                        }
                }
                echo ("<td valign=\"default\" bgcolor=\"#CCCCCC\">");
                echo ("<div align=\"center\"><b>".$row_totals[$rt++]."</b></div></td>\n");
                echo ("</tr>\n");
                echo ("<tr bgcolor=\"#CCCCCC\">\n");
                echo ("<td height=\"30\">\n");
                echo ("<div align=\"center\"><strong>TOTALS</strong></div></td>\n");

                for ($f = 0; $f < $cids_col_count; $f++){
                        for ($g = 0; $g < $cids_row_count; $g++){
                                if ($g == 0){
                                        $subtotal = $f;
                                        $total = intval($col_counts[$f]);
                                }
                                else {
                                        $subtotal += $cids_col_count;
                                        $total += intval($col_counts[$subtotal]);
                                }
                        }
                        echo ("<td valign=\"default\">");
                        echo ("<div align=\"center\"><strong>".$total."</strong></div></td>\n");
                        $grand_total += $total;
                        unset($total);
                }
                echo ("<td valign=\"default\">");
                echo ("<div align=\"center\"><strong>".$grand_total."</strong></div></td>\n");
                echo ("</tr>\n");
?>
                </table>
		 <p>&nbsp;</p>  
		<a href="<?php echo($ESPCONFIG['base_url']); ?>index.php?where=results&sid=<?php echo($sid);?>&type=cross">Go back to Cross Analysis/Tabulation</a>
		</center>
		
<?php
  }
?>
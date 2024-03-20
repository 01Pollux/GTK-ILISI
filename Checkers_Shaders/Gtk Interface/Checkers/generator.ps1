$CellSize = 80;
$Width = $CellSize * 8;
$Height = $CellSize * 8 - 10;

$board = @(
    @(1,0,1,0,1,0,1,0),
    @(0,1,0,1,0,1,0,1),
    @(1,0,1,0,1,0,1,0),
    @(0,0,0,0,0,0,0,0),
    @(0,0,0,0,0,0,0,0),
    @(0,2,0,2,0,2,0,2),
    @(4,0,2,0,2,0,2,0),
    @(0,2,0,2,0,2,0,2)
);

$Header = @"
<Application>
	<Window Size="${Width}x${Height}" Title="" Id="SecondWindow">
		<Headerbar Title=" " Layout="icon:close">
			<Button Pack="End" Type="Label" Text=">"></Button>
			<Button Pack="End" Type="Label" Text="<"></Button>
		</Headerbar>
		<Box Fixed="true" Id="MainBox">
			<Grid Fill="true" Expand="true">
                <Private>
                    <Align Vertical="Start"></Align>
                </Private>

"@;

$Footer = @"
        </Box>
    </Window>
</Application>
"@;

$Color = "Black";
$Invert = $false;

for ($i = 0; $i -lt 8; $i++) {
    $Header += '                <GridTr>' + "`r`n";
    for ($j = 0; $j -lt 8; $j++) {
        $Header += [String]::Format('                    <GridTd Width="1" Height="1"><Image Path="Checkers/{0}.png" Size="{1}x{1}"></Image></GridTd>' + "`r`n", $Color, $CellSize);
        if ($Color -eq 'Black' )
        {
            $Color = 'Grey';
        }
        else 
        {
            $Color = 'Black';
        }
    }
    if ($Color -eq 'Black' )
        {
            $Color = 'Grey';
        }
        else 
        {
            $Color = 'Black';
        }
    $Header += '                </GridTr>' + "`r`n";
}

$Header += "            </Grid>" + "`r`n";;

$idb = 0;
$idw = 0;
$last_id = 1;
for ($i = 0; $i -lt 8; $i++)
{
    for ($j = 0; $j -lt 8; $j++)
    {
        $insert = $false;
        switch ($board[$i][$j])
        {
            1 {
                $name = "Pawn_b" + $idb;
                $insert = $true;
                $path = "Checkers/pblack.png";
                $idb++;
            }
            2 {
                $name = "Pawn_w" + $idw;
                $insert = $true;
                $path = "Checkers/pred.png";
                $idw++;
            }
            3 {
                $name = "Pawn_b" + $idb;
				$insert = $true;
				$path = "Checkers/pblackq.png";
				$idb++;
			}
            4 {
				$name = "Pawn_w" + $idw;
				$insert = $true;
				$path = "Checkers/predq.png";
                $idw++;
			}
        }

        if ($insert)
        {
            $x = $i * $CellSize;
            $y = $j * $CellSize;

            $Header += [String]::Format('            <Image Path="{0}" Id="{1}" Size="{4}x{4}" FixedPosition="{2},{3}"></Image>' + "`r`n", $path, $name, $y, $x, $CellSize);
        }
    }
}

$Header += @"
            <Image Path="Checkers/predf.png" Id="Pawn_f" Size="${CellSize}x${CellSize}"><Private><Hidden></Hidden></Private></Image>

"@;
$Header += $Footer;

Out-File -FilePath "Table.xml" -InputObject $Header -Encoding UTF8 -Force
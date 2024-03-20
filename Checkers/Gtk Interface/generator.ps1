$Header = @"
<Application>
	<Window Size="800x800" Title="" Id="SecondWindow">
		<Headerbar Layout="menu:close">
		</Headerbar>
		<Box>
			<Grid Fill="true" Expand="true">
                <Private>
                    <Style Name="CheckerGrid"></Style>
                </Private>

"@;

$Footer = @"
            </Grid>
        </Box>
    </Window>
</Application>
"@;

for ($i = 0; $i -lt 3; $i++) {
    $Header += '                <GridTr>' + "`r`n";
    for ($j = 0; $j -lt 4; $j++) {
        $Header += [String]::Format('                    <GridTd Width="1" Height="1"><Button Id="Button[{0}][{1}]"></Button></GridTd>' + "`r`n", $i, $j);
    }
    $Header += '                </GridTr>' + "`r`n";
}

$Header += $Footer;

Out-File -FilePath "Testt2.xml" -InputObject $Header -Encoding UTF8 -Force
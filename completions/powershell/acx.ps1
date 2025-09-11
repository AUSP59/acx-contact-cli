# PowerShell tab-completion for acx
Register-ArgumentCompleter -CommandName acx -ScriptBlock {
    param($commandName, $parameterName, $wordToComplete, $commandAst, $fakeBoundParameters)
    $subs = @('help','list','import','export','validate','stats','version')
    $subs | Where-Object { $_ -like "$wordToComplete*" } | ForEach-Object {
        [System.Management.Automation.CompletionResult]::new($_, $_, 'ParameterValue', $_)
    }
}

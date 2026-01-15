# Install Qt and remaining dependencies for RedkaConnect build
Write-Host "Installing Qt 6.4.2..."

# Download Qt installer
$qtInstallerUrl = "https://d13lb3tujbc63x.cloudfront.net/qt-installer-framework/4.6/qt-installer-framework-4.6.0-windows-x86.exe"
$qtInstallerPath = "$env:TEMP\qt-installer.exe"

Write-Host "Downloading Qt installer..."
Invoke-WebRequest -Uri $qtInstallerUrl -OutFile $qtInstallerPath

# Create Qt installation script
$qtScript = @"
function Controller() {
    installer.autoRejectMessageBoxes();
    installer.installationFinished.connect(function() {
        gui.clickButton(buttons.NextButton);
    });
}

Controller.prototype.WelcomePageCallback = function() {
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.CredentialsPageCallback = function() {
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.IntroductionPageCallback = function() {
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.TargetDirectoryPageCallback = function() {
    gui.currentPageWidget().TargetDirectoryLineEdit.setText("C:/Qt");
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.ComponentSelectionPageCallback = function() {
    var widget = gui.currentPageWidget();
    widget.selectComponent("qt.qt6.642.win64_msvc2019_64");
    widget.selectComponent("qt.qt6.642.qtnetworkauth");
    widget.selectComponent("qt.qt6.642.qtserialport");
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.LicenseAgreementPageCallback = function() {
    gui.currentPageWidget().AcceptLicenseRadioButton.setChecked(true);
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.StartMenuDirectoryPageCallback = function() {
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.ReadyForInstallationPageCallback = function() {
    gui.clickButton(buttons.NextButton);
};

Controller.prototype.FinishedPageCallback = function() {
    var checkBoxForm = gui.currentPageWidget().LaunchQtCreatorCheckBoxForm;
    if (checkBoxForm && checkBoxForm.launchQtCreatorCheckBox) {
        checkBoxForm.launchQtCreatorCheckBox.checked = false;
    }
    gui.clickButton(buttons.FinishButton);
};
"@

$qtScriptPath = "$env:TEMP\qt-install-script.qs"
$qtScript | Out-File -FilePath $qtScriptPath -Encoding ASCII

Write-Host "Running Qt installer..."
Start-Process -FilePath $qtInstallerPath -ArgumentList "--script=$qtScriptPath" -Wait

# Clean up
Remove-Item $qtInstallerPath
Remove-Item $qtScriptPath

Write-Host "Qt installation completed!"
Write-Host ""
Write-Host "Now you can build RedkaConnect:"
Write-Host "1. Open PowerShell as Administrator"
Write-Host "2. cd C:\Users\exodo\Downloads\input-leap-master"
Write-Host "3. .\clean_build.ps1"
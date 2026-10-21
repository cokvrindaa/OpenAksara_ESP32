Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$form = New-Object System.Windows.Forms.Form
$form.Text = "OpenAksara - Waypoint Auto-Updater"
$form.Size = New-Object System.Drawing.Size(660, 490)
$form.StartPosition = "CenterScreen"
$form.BackColor = [System.Drawing.Color]::FromArgb(11, 13, 18)
$form.ForeColor = [System.Drawing.Color]::FromArgb(242, 244, 248)
$form.FormBorderStyle = [System.Windows.Forms.FormBorderStyle]::FixedDialog
$form.MaximizeBox = $false

$lblTitle = New-Object System.Windows.Forms.Label
$lblTitle.Location = New-Object System.Drawing.Point(20, 15)
$lblTitle.Size = New-Object System.Drawing.Size(600, 25)
$lblTitle.Text = "Paste Kode StrokeWaypoint Dari Browser Di Bawah Ini:"
$lblTitle.Font = New-Object System.Drawing.Font("Segoe UI", 11, [System.Drawing.FontStyle]::Bold)
$lblTitle.ForeColor = [System.Drawing.Color]::FromArgb(56, 189, 248)
$form.Controls.Add($lblTitle)

$txtCode = New-Object System.Windows.Forms.TextBox
$txtCode.Location = New-Object System.Drawing.Point(20, 48)
$txtCode.Size = New-Object System.Drawing.Size(605, 260)
$txtCode.Multiline = $true
$txtCode.ScrollBars = [System.Windows.Forms.ScrollBars]::Vertical
$txtCode.Font = New-Object System.Drawing.Font("Consolas", 10)
$txtCode.BackColor = [System.Drawing.Color]::FromArgb(22, 26, 34)
$txtCode.ForeColor = [System.Drawing.Color]::FromArgb(34, 197, 94)
$form.Controls.Add($txtCode)

$lblStatus = New-Object System.Windows.Forms.Label
$lblStatus.Location = New-Object System.Drawing.Point(20, 318)
$lblStatus.Size = New-Object System.Drawing.Size(605, 30)
$lblStatus.Text = "Status: Siap menerima kode dari clipboard..."
$lblStatus.Font = New-Object System.Drawing.Font("Segoe UI", 10)
$lblStatus.ForeColor = [System.Drawing.Color]::FromArgb(138, 147, 166)
$form.Controls.Add($lblStatus)

$btnUpdate = New-Object System.Windows.Forms.Button
$btnUpdate.Location = New-Object System.Drawing.Point(20, 365)
$btnUpdate.Size = New-Object System.Drawing.Size(280, 48)
$btnUpdate.Text = "🚀 OKE / UPDATE KODE C++"
$btnUpdate.Font = New-Object System.Drawing.Font("Segoe UI", 11, [System.Drawing.FontStyle]::Bold)
$btnUpdate.BackColor = [System.Drawing.Color]::FromArgb(37, 99, 235)
$btnUpdate.ForeColor = [System.Drawing.Color]::White
$btnUpdate.FlatStyle = [System.Windows.Forms.FlatStyle]::Flat
$form.Controls.Add($btnUpdate)

$btnClear = New-Object System.Windows.Forms.Button
$btnClear.Location = New-Object System.Drawing.Point(315, 365)
$btnClear.Size = New-Object System.Drawing.Size(130, 48)
$btnClear.Text = "🗑 Bersihkan"
$btnClear.Font = New-Object System.Drawing.Font("Segoe UI", 10)
$btnClear.BackColor = [System.Drawing.Color]::FromArgb(35, 41, 56)
$btnClear.ForeColor = [System.Drawing.Color]::White
$btnClear.FlatStyle = [System.Windows.Forms.FlatStyle]::Flat
$form.Controls.Add($btnClear)

$btnOpenHtml = New-Object System.Windows.Forms.Button
$btnOpenHtml.Location = New-Object System.Drawing.Point(460, 365)
$btnOpenHtml.Size = New-Object System.Drawing.Size(165, 48)
$btnOpenHtml.Text = "🌐 Buka HTML Plotter"
$btnOpenHtml.Font = New-Object System.Drawing.Font("Segoe UI", 10)
$btnOpenHtml.BackColor = [System.Drawing.Color]::FromArgb(35, 41, 56)
$btnOpenHtml.ForeColor = [System.Drawing.Color]::FromArgb(56, 189, 248)
$btnOpenHtml.FlatStyle = [System.Windows.Forms.FlatStyle]::Flat
$form.Controls.Add($btnOpenHtml)

$btnOpenHtml.Add_Click({
  $htmlPath = Join-Path $PSScriptRoot "waypoint_editor.html"
  if (Test-Path $htmlPath) {
    Start-Process $htmlPath
  }
})

$btnClear.Add_Click({
  $txtCode.Text = ""
  $lblStatus.Text = "Status: Kotak teks dibersihkan."
  $lblStatus.ForeColor = [System.Drawing.Color]::FromArgb(138, 147, 166)
})

$btnUpdate.Add_Click({
  $code = $txtCode.Text.Trim()
  if ([string]::IsNullOrWhiteSpace($code)) {
    $lblStatus.Text = "⚠️ Kotak teks masih kosong! Silakan paste kode dulu."
    $lblStatus.ForeColor = [System.Drawing.Color]::FromArgb(239, 68, 68)
    return
  }

  if ($code -match 'StrokeWaypoint\s+([A-Z0-9_]+)\[\]\s*=') {
    $varName = $matches[1]
  } else {
    $lblStatus.Text = "❌ Format kode salah! Harus ada 'StrokeWaypoint NAMA[] = ...'"
    $lblStatus.ForeColor = [System.Drawing.Color]::FromArgb(239, 68, 68)
    return
  }

  $targetFile = ""
  if ($varName.StartsWith("BALI_")) {
    $targetFile = Join-Path $PSScriptRoot "src\ui_bali.cpp"
  } elseif ($varName.StartsWith("JAWA_")) {
    $targetFile = Join-Path $PSScriptRoot "src\ui_jawa.cpp"
  } else {
    $lblStatus.Text = "❌ Nama variabel '$varName' harus diawali BALI_ atau JAWA_"
    $lblStatus.ForeColor = [System.Drawing.Color]::FromArgb(239, 68, 68)
    return
  }

  if (-not (Test-Path $targetFile)) {
    $lblStatus.Text = "❌ File target tidak ditemukan: $targetFile"
    $lblStatus.ForeColor = [System.Drawing.Color]::FromArgb(239, 68, 68)
    return
  }

  $content = [System.IO.File]::ReadAllText($targetFile)
  $pattern = "(?s)static\s+const\s+StrokeWaypoint\s+" + [regex]::Escape($varName) + "\[\]\s*=\s*\{.*?\};"

  if ($content -match $pattern) {
    $newContent = [regex]::Replace($content, $pattern, $code)
    [System.IO.File]::WriteAllText($targetFile, $newContent)
    $fileName = Split-Path $targetFile -Leaf
    $lblStatus.Text = "✅ SUKSES! '$varName' berhasil diupdate di $fileName"
    $lblStatus.ForeColor = [System.Drawing.Color]::FromArgb(34, 197, 94)
    [System.Windows.Forms.MessageBox]::Show("Berhasil mengupdate $varName di $fileName!`nKode C++ proyek Anda sudah otomatis terisi.", "Update Berhasil!", [System.Windows.Forms.MessageBoxButtons]::OK, [System.Windows.Forms.MessageBoxIcon]::Information)
  } else {
    $anchor = if ($varName.StartsWith("BALI_")) { "static void get_bali_guide" } else { "static void get_jawa_guide" }
    if ($content.Contains($anchor)) {
      $newContent = $content.Replace($anchor, "$code`r`n`r`n$anchor")
      [System.IO.File]::WriteAllText($targetFile, $newContent)
      $fileName = Split-Path $targetFile -Leaf
      $lblStatus.Text = "✅ SUKSES! '$varName' baru berhasil ditambahkan ke $fileName"
      $lblStatus.ForeColor = [System.Drawing.Color]::FromArgb(34, 197, 94)
      [System.Windows.Forms.MessageBox]::Show("Variabel baru $varName berhasil ditambahkan ke $fileName!`nKode C++ proyek Anda sudah otomatis terisi.", "Penambahan Berhasil!", [System.Windows.Forms.MessageBoxButtons]::OK, [System.Windows.Forms.MessageBoxIcon]::Information)
    } else {
      $lblStatus.Text = "⚠️ Variabel '$varName' tidak ditemukan dalam file $fileName"
      $lblStatus.ForeColor = [System.Drawing.Color]::FromArgb(245, 158, 11)
    }
  }
})

$form.ShowDialog() | Out-Null


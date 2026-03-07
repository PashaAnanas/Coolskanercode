# Читаем простой текстовый файл
data = []
with open('scan_results.txt', 'r', encoding='utf-8') as f:
    for line in f:
        parts = line.strip().split('|')
        if len(parts) == 6:
            data.append({
                'file': parts[0],
                'line_num': int(parts[1]),
                'type': parts[2],
                'value': parts[3],
                'entropy': float(parts[4]),
                'content': parts[5]
            })

files = {}
for item in data:
    if item['file'] not in files:
        files[item['file']] = []
    files[item['file']].append(item)

total = len(data)
by_type = {}
for item in data:
    by_type[item['type']] = by_type.get(item['type'], 0) + 1

html = f"""
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Secret Scanner Report</title>
    <style>
        body {{ font-family: Arial; margin: 20px; background: #f0f0f0; }}
        .container {{ max-width: 1200px; margin: 0 auto; background: white; padding: 20px; }}
        h1 {{ color: #d9534f; }}
        .stats {{ background: #f8f9fa; padding: 15px; margin: 20px 0; }}
        .file {{ margin: 20px 0; border: 1px solid #ddd; }}
        .file-header {{ background: #f8f9fa; padding: 10px; cursor: pointer; }}
        .file-content {{ padding: 10px; display: none; }}
        .file-content.show {{ display: block; }}
        .line {{ margin: 10px 0; padding: 10px; background: #f8f9fa; }}
        .line-number {{ color: #d9534f; font-weight: bold; }}
        .code {{ font-family: monospace; background: #272822; color: #f8f8f2; padding: 10px; }}
        .tag {{ display: inline-block; padding: 2px 8px; margin: 2px; border-radius: 3px; }}
        .tag-keywords {{ background: #ffd966; }}
        .tag-entropy {{ background: #f4a261; }}
        .tag-search {{ background: #e9c46a; }}
    </style>
</head>
<body>
    <div class="container">
        <h1>🔍 Secret Scanner Report</h1>
        
        <div class="stats">
            <p><strong>Всего находок:</strong> {total}</p>
            <p><strong>По типам:</strong></p>
            <ul>
"""

for t, c in by_type.items():
    html += f"<li>{t}: {c}</li>\n"

html += """
            </ul>
        </div>
"""

for filename, items in files.items():
    html += f"""
        <div class="file">
            <div class="file-header" onclick="this.nextElementSibling.classList.toggle('show')">
                📁 {filename} <span style="float: right;">▼</span>
            </div>
            <div class="file-content">
    """
    
    for item in items:
        html += f"""
                <div class="line">
                    <div class="line-number">Строка {item['line_num']}</div>
                    <div class="code">{item['content']}</div>
                    <div>
                        <span class="tag tag-{item['type']}">{item['type']}: {item['value']}</span>
                        {f'<span class="tag">Энтропия: {item["entropy"]:.2f}</span>' if item['type'] == 'entropy' else ''}
                    </div>
                </div>
        """
    
    html += """
            </div>
        </div>
    """

html += """
    </div>
    <script>
        function expandAll() {
            document.querySelectorAll('.file-content').forEach(el => el.classList.add('show'));
        }
        function collapseAll() {
            document.querySelectorAll('.file-content').forEach(el => el.classList.remove('show'));
        }
    </script>
    <div style="text-align: center; margin: 20px;">
        <button onclick="expandAll()">Развернуть все</button>
        <button onclick="collapseAll()">Свернуть все</button>
    </div>
</body>
</html>
"""

with open('secret_report.html', 'w', encoding='utf-8') as f:
    f.write(html)

print(f"✅ Отчет создан! Найдено {total} секретов.")
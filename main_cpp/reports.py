import json
from datetime import datetime

# Читаем данные
with open('scan_results.json', 'r', encoding='utf-8') as f:
    data = json.load(f)

# Считаем статистику
total_secrets = 0
keyword_count = 0
entropy_count = 0
search_count = 0

for file in data['files']:
    for line in file['lines']:
        total_secrets += len(line['secrets'])
        for secret in line['secrets']:
            if secret['type'] == 'keywords':
                keyword_count += 1
            elif secret['type'] == 'entropy':
                entropy_count += 1
            elif secret['type'] == 'search':
                search_count += 1

# Генерируем HTML
html = f"""
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Secret Scanner - Отчет</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            margin: 20px;
            background: #f5f5f5;
        }}
        .container {{
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }}
        h1 {{
            color: #333;
            border-bottom: 3px solid #d9534f;
            padding-bottom: 10px;
        }}
        .stats {{
            display: flex;
            gap: 15px;
            margin: 20px 0;
            flex-wrap: wrap;
        }}
        .stat-box {{
            background: #f8f9fa;
            padding: 15px 25px;
            border-radius: 5px;
            border-left: 4px solid #d9534f;
        }}
        .stat-box span {{
            font-size: 24px;
            font-weight: bold;
            color: #d9534f;
        }}
        .file-section {{
            margin-top: 30px;
            border: 1px solid #ddd;
            border-radius: 5px;
        }}
        .file-header {{
            background: #f8f9fa;
            padding: 12px 15px;
            cursor: pointer;
            font-weight: bold;
            border-bottom: 1px solid #ddd;
        }}
        .file-header:hover {{
            background: #e9ecef;
        }}
        .file-content {{
            padding: 15px;
            display: none;
        }}
        .file-content.show {{
            display: block;
        }}
        .line {{
            margin: 15px 0;
            padding: 10px;
            background: #f8f9fa;
            border-radius: 5px;
        }}
        .line-number {{
            color: #d9534f;
            font-weight: bold;
        }}
        .code {{
            font-family: monospace;
            background: #272822;
            color: #f8f8f2;
            padding: 10px;
            border-radius: 5px;
            margin: 8px 0;
            overflow-x: auto;
        }}
        .secrets {{
            display: flex;
            gap: 8px;
            flex-wrap: wrap;
        }}
        .secret-tag {{
            padding: 3px 10px;
            border-radius: 15px;
            font-size: 12px;
            font-weight: bold;
        }}
        .tag-keywords {{ background: #ffd966; color: #000; }}
        .tag-entropy {{ background: #f4a261; color: #000; }}
        .tag-search {{ background: #e9c46a; color: #000; }}
        .summary {{
            background: #f8f9fa;
            padding: 15px;
            border-radius: 5px;
            margin: 20px 0;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>🔍 Secret Scanner - Отчет о сканировании</h1>
        <p>Время сканирования: {data['scan_time']}</p>
        
        <div class="stats">
            <div class="stat-box">
                <div>Всего файлов</div>
                <span>{data['total_files']}</span>
            </div>
            <div class="stat-box">
                <div>Найдено секретов</div>
                <span>{total_secrets}</span>
            </div>
            <div class="stat-box">
                <div>Ключевые слова</div>
                <span>{keyword_count}</span>
            </div>
            <div class="stat-box">
                <div>Высокая энтропия</div>
                <span>{entropy_count}</span>
            </div>
            <div class="stat-box">
                <div>Regex поиск</div>
                <span>{search_count}</span>
            </div>
        </div>
        
        <div class="summary">
            <strong>Всего найдено подозрительных мест:</strong> {total_secrets}
        </div>
"""

# Добавляем файлы
for file in data['files']:
    has_secrets = False
    for line in file['lines']:
        if line.get('secrets'):
            has_secrets = True
            break
    
    if not has_secrets:
        continue
        
    html += f"""
        <div class="file-section">
            <div class="file-header" onclick="this.nextElementSibling.classList.toggle('show')">
                📁 {file['filename']} <span style="float: right;">▼</span>
            </div>
            <div class="file-content">
    """
    
    for line in file['lines']:
        if not line.get('secrets'):
            continue
            
        html += f"""
                <div class="line">
                    <div class="line-number">Строка {line['line_number']}</div>
                    <div class="code">{line['content']}</div>
                    <div class="secrets">
        """
        
        for secret in line['secrets']:
            if secret['type'] == 'keywords':
                html += f'<span class="secret-tag tag-keywords">🔑 {secret["value"]}</span>'
            elif secret['type'] == 'entropy':
                html += f'<span class="secret-tag tag-entropy">🎲 Энтропия: {secret["entropy"]:.2f}</span>'
            elif secret['type'] == 'search':
                html += f'<span class="secret-tag tag-search">🔍 {secret["value"]}</span>'
        
        html += """
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
        // Можно открыть все файлы сразу
        function expandAll() {
            document.querySelectorAll('.file-content').forEach(el => el.classList.add('show'));
        }
        function collapseAll() {
            document.querySelectorAll('.file-content').forEach(el => el.classList.remove('show'));
        }
    </script>
    
    <div style="text-align: center; margin-top: 20px;">
        <button onclick="expandAll()">Развернуть все</button>
        <button onclick="collapseAll()">Свернуть все</button>
    </div>
</body>
</html>
"""

# Сохраняем отчет
with open('secret_report.html', 'w', encoding='utf-8') as f:
    f.write(html)

print(f"""
=====================================
✅ ОТЧЕТ СОЗДАН!
=====================================
Файлов: {data['total_files']}
Секретов: {total_secrets}
  • Ключевые слова: {keyword_count}
  • Высокая энтропия: {entropy_count}
  • Regex: {search_count}
=====================================
Открыть файл: secret_report.html
""")
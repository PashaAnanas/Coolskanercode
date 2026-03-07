#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import sys
from datetime import datetime

def load_results(filename="scan_results.json"):
    """Загрузка результатов из JSON"""
    try:
        with open(filename, 'r') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"❌ Файл {filename} не найден")
        return None
    except json.JSONDecodeError:
        print(f"❌ Ошибка парсинга JSON")
        return None

def print_summary(data):
    """Вывод краткой статистики"""
    if not data:
        return
    
    print("\n" + "="*60)
    print("📊 КРАТКАЯ СТАТИСТИКА")
    print("="*60)
    
    total_secrets = 0
    for file in data.get('files', []):
        for line in file.get('lines', []):
            total_secrets += len(line.get('secrets', []))
    
    print(f"📁 Файлов: {data.get('total_files', 0)}")
    print(f"🔐 Секретов: {total_secrets}")
    print(f"⏰ Время: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

def print_detailed(data):
    """Детальный вывод"""
    if not data:
        return
    
    print("\n" + "="*60)
    print("🔍 ДЕТАЛЬНЫЙ ОТЧЕТ")
    print("="*60)
    
    for file in data.get('files', []):
        print(f"\n📁 Файл: {file.get('name', 'Unknown')}")
        print("-"*40)
        
        for line in file.get('lines', []):
            if line.get('secrets'):
                print(f"  📝 Строка {line.get('number')}: {line.get('content')}")
                for secret in line.get('secrets', []):
                    src = secret.get('source', 'unknown')
                    val = secret.get('value', '')
                    ent = secret.get('entropy', 0)
                    
                    if ent > 0:
                        print(f"    🔑 [{src}] {val} (энтропия: {ent:.2f})")
                    else:
                        print(f"    🔑 [{src}] {val}")
                print()

def export_to_html(data, filename="report.html"):
    """Экспорт в HTML"""
    html = """
    <!DOCTYPE html>
    <html>
    <head>
        <title>Отчет о сканировании</title>
        <style>
            body { font-family: Arial; margin: 20px; }
            .file { margin: 20px 0; padding: 10px; border: 1px solid #ccc; }
            .secret { margin-left: 20px; color: #c00; }
        </style>
    </head>
    <body>
    """
    
    html += f"<h1>Отчет о сканировании</h1>"
    html += f"<p>Всего файлов: {data.get('total_files', 0)}</p>"
    
    for file in data.get('files', []):
        html += f"<div class='file'>"
        html += f"<h3>{file.get('name')}</h3>"
        
        for line in file.get('lines', []):
            if line.get('secrets'):
                html += f"<p><b>Строка {line.get('number')}:</b> {line.get('content')}</p>"
                for secret in line.get('secrets', []):
                    html += f"<p class='secret'>🔑 [{secret.get('source')}] {secret.get('value')}</p>"
        
        html += "</div>"
    
    html += "</body></html>"
    
    with open(filename, 'w', encoding='utf-8') as f:
        f.write(html)
    
    print(f"✅ Отчет сохранен в {filename}")

if __name__ == "__main__":
    filename = sys.argv[1] if len(sys.argv) > 1 else "scan_results.json"
    
    data = load_results(filename)
    if data:
        print_summary(data)
        print_detailed(data)
        export_to_html(data)
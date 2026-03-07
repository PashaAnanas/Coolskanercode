from flask import Flask, render_template, jsonify, send_file, request
import json
import os
from datetime import datetime

app = Flask(__name__)

# Пути
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
MAIN_HTML = os.path.join(BASE_DIR, 'main.html')
JSON_FILE = os.path.join(BASE_DIR, 'scan_results.json')
HTML_FILE = os.path.join(BASE_DIR, 'scan_results.html')
TXT_FILE = os.path.join(BASE_DIR, 'scan_results.txt')

# Функция для загрузки данных из JSON
def load_results():
    if os.path.exists(JSON_FILE):
        try:
            with open(JSON_FILE, 'r', encoding='utf-8') as f:
                return json.load(f)
        except:
            return {"total_files": 0, "total_secrets": 0, "keywords": 0, "entropy": 0, "regex": 0}
    return {"total_files": 0, "total_secrets": 0, "keywords": 0, "entropy": 0, "regex": 0}

@app.route('/')
def index():
    """Главная страница - main.html"""
    return send_file(MAIN_HTML)

@app.route('/api/results')
def api_results():
    """API для получения результатов"""
    return jsonify(load_results())

@app.route('/scan', methods=['POST'])
def scan():
    """Запуск сканирования"""
    data = request.json
    path = data.get('path', '')
    
    # Здесь будет вызов C++ сканера
    # Пока просто эмулируем
    print(f"Сканирование: {path}")
    
    return jsonify({"status": "started", "path": path})

@app.route('/download/<filetype>')
def download(filetype):
    """Скачать файл"""
    if filetype == 'json' and os.path.exists(JSON_FILE):
        return send_file(JSON_FILE, as_attachment=True)
    elif filetype == 'html' and os.path.exists(HTML_FILE):
        return send_file(HTML_FILE, as_attachment=True)
    elif filetype == 'txt' and os.path.exists(TXT_FILE):
        return send_file(TXT_FILE, as_attachment=True)
    return "Файл не найден", 404

@app.route('/report')
def report():
    """Показать HTML отчет"""
    if os.path.exists(HTML_FILE):
        return send_file(HTML_FILE)
    return "Отчет не найден", 404

@app.route('/clear', methods=['POST'])
def clear():
    """Очистить результаты"""
    if os.path.exists(JSON_FILE):
        os.remove(JSON_FILE)
    return jsonify({"status": "cleared"})

if __name__ == '__main__':
    print("\n" + "="*60)
    print("🔥 КРУТОЙ СКАНЕР - ЗАПУЩЕН!")
    print("="*60)
    print(f"📁 main.html: {MAIN_HTML}")
    print(f"📁 JSON: {JSON_FILE}")
    print(f"\n📌 http://localhost:5000")
    print("="*60 + "\n")
    
    app.run(debug=True, host='127.0.0.1', port=5000)
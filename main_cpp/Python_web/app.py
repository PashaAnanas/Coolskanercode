from flask import Flask, render_template, request, jsonify, send_file
import json
import os
import subprocess
import webbrowser
from datetime import datetime
import socket

app = Flask(__name__)

# Конфигурация - АБСОЛЮТНЫЕ ПУТИ
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
SCAN_RESULTS_FILE = os.path.join(BASE_DIR, "..", "scan_results.json")
SCAN_TXT_FILE = os.path.join(BASE_DIR, "..", "scan_results.txt")
SCAN_HTML_FILE = os.path.join(BASE_DIR, "..", "scan_results.html")

def get_local_ip():
    """Получение локального IP адреса"""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except:
        return "127.0.0.1"

def check_files():
    """Проверка наличия файлов"""
    files = {
        "scan_results.json": SCAN_RESULTS_FILE,
        "scan_results.txt": SCAN_TXT_FILE,
        "scan_results.html": SCAN_HTML_FILE
    }
    
    print("\n📁 ПРОВЕРКА ФАЙЛОВ:")
    for name, path in files.items():
        if os.path.exists(path):
            size = os.path.getsize(path)
            print(f"  ✅ {name}: {size} байт")
        else:
            print(f"  ❌ {name}: НЕ НАЙДЕН")

@app.route('/')
def index():
    """Главная страница"""
    return render_template('input.html')

@app.route('/scan', methods=['POST'])
def run_scan():
    """Запуск C++ сканера"""
    try:
        # Путь к C++ сканеру
        scanner_path = os.path.join(BASE_DIR, "..", "main_cpp", "scanner.exe")
        
        if not os.path.exists(scanner_path):
            return render_template('results.html',
                                 error=True,
                                 message=f"❌ Сканер не найден: {scanner_path}")
        
        # Запускаем C++ программу
        result = subprocess.run([scanner_path], 
                               capture_output=True, text=True,
                               cwd=os.path.dirname(scanner_path))
        
        # Проверяем результаты
        if os.path.exists(SCAN_RESULTS_FILE):
            with open(SCAN_RESULTS_FILE, 'r', encoding='utf-8') as f:
                data = json.load(f)
            return render_template('results.html', 
                                 results=data,
                                 success=True,
                                 message="✅ Сканирование завершено")
        else:
            return render_template('results.html',
                                 error=True,
                                 message="❌ Файл результатов не найден")
    
    except Exception as e:
        return render_template('results.html',
                             error=True,
                             message=f"❌ Ошибка: {str(e)}")

@app.route('/load')
def load_results():
    """Загрузка существующих результатов"""
    try:
        if os.path.exists(SCAN_RESULTS_FILE):
            with open(SCAN_RESULTS_FILE, 'r', encoding='utf-8') as f:
                data = json.load(f)
            return render_template('results.html', results=data)
        else:
            return render_template('results.html', 
                                 error=True, 
                                 message="❌ Нет результатов сканирования")
    except Exception as e:
        return render_template('results.html',
                             error=True,
                             message=f"❌ Ошибка загрузки: {str(e)}")

@app.route('/api/results')
def api_results():
    """API для получения JSON"""
    try:
        if os.path.exists(SCAN_RESULTS_FILE):
            with open(SCAN_RESULTS_FILE, 'r', encoding='utf-8') as f:
                data = json.load(f)
            return jsonify(data)
        else:
            return jsonify({"error": "No results found"})
    except Exception as e:
        return jsonify({"error": str(e)})

@app.route('/download/<format>')
def download(format):
    """Скачать результаты в нужном формате"""
    files = {
        'json': SCAN_RESULTS_FILE,
        'txt': SCAN_TXT_FILE,
        'html': SCAN_HTML_FILE
    }
    
    if format in files and os.path.exists(files[format]):
        return send_file(files[format], as_attachment=True)
    return "❌ Файл не найден", 404

@app.route('/stats')
def stats():
    """Статистика сканирований"""
    stats_data = {
        "last_scan": None,
        "total_secrets": 0,
        "total_files": 0
    }
    
    if os.path.exists(SCAN_RESULTS_FILE):
        mod_time = os.path.getmtime(SCAN_RESULTS_FILE)
        stats_data["last_scan"] = datetime.fromtimestamp(mod_time).strftime('%Y-%m-%d %H:%M:%S')
        
        with open(SCAN_RESULTS_FILE, 'r', encoding='utf-8') as f:
            data = json.load(f)
            stats_data["total_files"] = data.get("total_files", 0)
            
            secrets = 0
            for file in data.get("files", []):
                for line in file.get("lines", []):
                    secrets += len(line.get("secrets", []))
            stats_data["total_secrets"] = secrets
    
    return render_template('stats.html', stats=stats_data)

@app.route('/open-browser')
def open_browser():
    """Открыть браузер с локальным адресом"""
    ip = get_local_ip()
    webbrowser.open(f"http://{ip}:5000")
    return "✅ Браузер открыт"

@app.route('/debug')
def debug():
    """Отладочная информация"""
    info = {
        "base_dir": BASE_DIR,
        "scan_results_json": str(SCAN_RESULTS_FILE),
        "json_exists": os.path.exists(SCAN_RESULTS_FILE),
        "txt_exists": os.path.exists(SCAN_TXT_FILE),
        "html_exists": os.path.exists(SCAN_HTML_FILE),
        "python_version": os.sys.version,
        "ip": get_local_ip()
    }
    
    if info["json_exists"]:
        info["json_size"] = os.path.getsize(SCAN_RESULTS_FILE)
    
    return jsonify(info)

@app.context_processor
def utility_processor():
    def now():
        return datetime.now()
    return dict(now=now)

if __name__ == '__main__':
    ip = get_local_ip()
    
    print("\n" + "="*60)
    print("🌐 КРУТОЙ СКАНЕР - ВЕБ-ИНТЕРФЕЙС")
    print("="*60)
    print(f"\n📁 Базовая папка: {BASE_DIR}")
    check_files()
    print(f"\n📌 Локальный доступ: http://localhost:5000")
    print(f"📌 В сети: http://{ip}:5000")
    print(f"📌 Отладка: http://localhost:5000/debug")
    print(f"\n🔧 Для остановки: Ctrl+C")
    print("="*60 + "\n")
    
    # Открываем браузер автоматически
    webbrowser.open(f"http://localhost:5000")
    
    # ВАЖНО: включаем хост и отладку
    app.run(debug=True, host='0.0.0.0', port=5000)
from flask import Flask, render_template
import webbrowser
from threading import Timer
import os

app = Flask(__name__)

def read_data():
    """Читает простой текстовый файл"""
    results = []
    current_file = {}
    current_line = {}
    
    if os.path.exists('scan_results_simple.txt'):
        with open('scan_results_simple.txt', 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if line.startswith('FILE:'):
                    current_file = {'filename': line[5:].strip(), 'lines': []}
                    results.append(current_file)
                elif line.startswith('LINE:'):
                    current_line = {'number': line[5:].strip(), 'content': '', 'secrets': []}
                    current_file['lines'].append(current_line)
                elif line.startswith('CONTENT:'):
                    if current_line:
                        current_line['content'] = line[8:].strip()
                elif line.startswith('  SECRET:'):
                    if current_line:
                        current_line['secrets'].append(line[9:].strip())
    
    return results

def open_browser():
    webbrowser.open('http://127.0.0.1:5000/')

@app.route('/')
def index():
    data = read_data()
    return render_template('simple_results.html', data=data)

if __name__ == '__main__':
    Timer(1, open_browser).start()
    app.run(debug=True, port=5000)
from flask import Flask, render_template, request, redirect, url_for, session
import json
import os
import subprocess

app = Flask(__name__)
app.secret_key = 'nitro_secret_key'

CONFIG_FILE = 'config.json'
APP_BINARY = './build/cam_app'


def load_config():
    default = {
        "resolution": [1280, 720], 
        "color": [255, 255, 255], 
        "osd_text": "Default", 
        "compass_speed": 1.0,
        "camera_path": " ",
        "is_comp_ccw": False
    }

    if not os.path.exists(CONFIG_FILE):
        return default
    
    with open(CONFIG_FILE, 'r') as f:
        config = json.load(f)
        if 'color' in config:
            config['color'] = [int(c) for c in config['color']]
        return {**default, **config}


def hex_to_bgr(hex_str):
    hex_str = hex_str.lstrip('#')
    r = int(hex_str[0:2], 16)
    g = int(hex_str[2:4], 16)
    b = int(hex_str[4:6], 16)
    return [b, g, r]


@app.route('/', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        if request.form.get('username') == 'admin':
            session['logged_in'] = True
            return redirect(url_for('settings'))
    return render_template('login.html')

@app.route('/settings', methods=['GET', 'POST'])
def settings():
    if not session.get('logged_in'):
        return redirect(url_for('login'))

    config = load_config()

    if request.method == 'POST':
        res = request.form.get('resolution', '1280x720')
        w, h = map(int, res.split('x'))
        speed_raw = request.form.get('speed')

        config.update({
            "resolution": [w,h],
            "color": hex_to_bgr(request.form.get('color_hex', '#00ff00')),
            "osd_text": request.form.get('osd_text', 'Default') or "Default",
            "compass_speed": float(speed_raw) if speed_raw else 1.0,
        })        

        with open(CONFIG_FILE, 'w') as f:
            json.dump(config, f, indent=4)

        cmd = [APP_BINARY, f"-c={config.get('cam_path')}"]
        if config.get('is_comp_ccw', False):
            cmd.append("-cr")

        subprocess.run(["pkill", "-f", APP_BINARY], check=False)
        subprocess.Popen(cmd)

        return redirect(url_for('settings'))

    return render_template('settings.html', config=config)


if __name__ == '__main__':
    app.run(debug=True, port=5000)
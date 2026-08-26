"use client";

import { useEffect, useState } from "react";

type IconName = "pulse" | "lungs" | "motion" | "brain" | "wifi" | "battery" | "bell" | "shield" | "clock" | "chip";

function Icon({ name, size = 18 }: { name: IconName; size?: number }) {
  const shapes: Record<IconName, React.ReactNode> = {
    pulse: <path d="M3 12h4l2.2-6.5 4 13 2.2-6.5H21" />,
    lungs: <><path d="M12 4v8"/><path d="M9 6.5C5.3 8.1 3 11.3 3 16c0 2.4 1.5 4 3.8 4 3.2 0 5.2-3.5 5.2-8"/><path d="M15 6.5c3.7 1.6 6 4.8 6 9.5 0 2.4-1.5 4-3.8 4-3.2 0-5.2-3.5-5.2-8"/></>,
    motion: <><circle cx="12" cy="5" r="2"/><path d="m9 22 1-7-3-2 3-5 4 2 3-1M14 22l-1-6 3-2"/></>,
    brain: <><path d="M9.5 4A3 3 0 0 0 6 7a3 3 0 0 0-1 5.8A3.5 3.5 0 0 0 8.5 19H10V5.5A1.5 1.5 0 0 0 9.5 4Z"/><path d="M14.5 4A3 3 0 0 1 18 7a3 3 0 0 1 1 5.8 3.5 3.5 0 0 1-3.5 6.2H14V5.5a1.5 1.5 0 0 1 .5-1.5Z"/><path d="M6 8.5h2M16 8.5h2M6 15h2m8 0h2"/></>,
    wifi: <><path d="M5 12.5a10 10 0 0 1 14 0M8.5 16a5 5 0 0 1 7 0"/><circle cx="12" cy="20" r=".7" fill="currentColor"/></>,
    battery: <><rect x="3" y="7" width="17" height="10" rx="2"/><path d="M22 11v2M6 10v4m3-4v4m3-4v4m3-4v4"/></>,
    bell: <><path d="M18 8a6 6 0 0 0-12 0c0 7-3 7-3 9h18c0-2-3-2-3-9M10 21h4"/></>,
    shield: <><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10Z"/><path d="m9 12 2 2 4-4"/></>,
    clock: <><circle cx="12" cy="12" r="9"/><path d="M12 7v5l3 2"/></>,
    chip: <><rect x="6" y="6" width="12" height="12" rx="2"/><rect x="9" y="9" width="6" height="6"/><path d="M9 2v4m6-4v4M9 18v4m6-4v4M2 9h4m-4 6h4m12-6h4m-4 6h4"/></>,
  };
  return <svg width={size} height={size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.7" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">{shapes[name]}</svg>;
}

const heartPath = "M0 45 C18 44 22 45 34 43 L42 43 L47 31 L53 58 L60 13 L68 66 L76 42 C90 42 96 45 112 43 C127 42 139 45 151 43 L159 42 L166 28 L171 57 L179 12 L187 65 L196 42 C211 43 223 44 236 42 C250 42 260 45 273 43 L281 42 L288 30 L294 58 L302 15 L310 65 L319 42 C336 43 346 44 360 42";
const breathPath = "M0 49 C27 49 40 48 52 42 C72 32 89 23 110 23 C132 23 148 34 165 42 C179 49 194 50 216 49 C243 49 255 48 269 41 C288 31 304 23 325 23 C343 23 350 30 360 35";
const motionBars = [22, 35, 29, 52, 76, 46, 34, 64, 42, 28, 36, 68, 55, 32, 26, 48, 74, 58, 40, 30, 44, 67, 51, 35];

function Sparkline({ path, color, fillId }: { path: string; color: string; fillId: string }) {
  return <svg viewBox="0 0 360 78" preserveAspectRatio="none" className="sparkline"><defs><linearGradient id={fillId} x1="0" y1="0" x2="0" y2="1"><stop stopColor={color} stopOpacity=".28"/><stop offset="1" stopColor={color} stopOpacity="0"/></linearGradient></defs><path d={`${path} L360 78 L0 78Z`} fill={`url(#${fillId})`}/><path d={path} stroke={color} strokeWidth="2" fill="none" vectorEffect="non-scaling-stroke"/></svg>;
}

function VitalCard({ type, label, value, unit, sensor, tone, children }: { type: IconName; label: string; value: number; unit: string; sensor: string; tone: string; children: React.ReactNode }) {
  return <article className="vital-card" style={{ "--tone": tone } as React.CSSProperties}>
    <div className="card-top"><span className="vital-icon"><Icon name={type}/></span><span className="live"><i/>LIVE</span></div>
    <p className="vital-label">{label}</p>
    <div className="vital-number">{value}<span>{unit}</span></div>
    <div className="chart-slot">{children}</div>
    <div className="card-foot"><span><i/>Signal stable</span><span>{sensor}</span></div>
  </article>;
}

export default function Dashboard() {
  const [tick, setTick] = useState(0);
  const [range, setRange] = useState("1H");
  const [notified, setNotified] = useState(false);
  useEffect(() => { const timer = window.setInterval(() => setTick(t => t + 1), 2400); return () => window.clearInterval(timer); }, []);
  const now = new Intl.DateTimeFormat("en-IN", { hour: "2-digit", minute: "2-digit", second: "2-digit" }).format(new Date());
  const heart = [94, 95, 93, 96][tick % 4];
  const respiration = [19, 19, 20, 19][tick % 4];
  const mobility = [82, 81, 82, 83][tick % 4];

  return <main className="app-shell">
    <header>
      <a className="brand" href="#"><span><Icon name="pulse" size={22}/></span><div>SENTRA<small>EARLY WARNING SYSTEM</small></div></a>
      <div className="patient"><div className="avatar">A</div><div><strong>Demo Patient</strong><span>MYOSA-SENTRA-01 · Continuous monitoring</span></div></div>
      <div className="header-actions"><div className="online"><i/>SYSTEM ONLINE</div><button aria-label="Notifications" onClick={() => setNotified(!notified)} className={notified ? "active" : ""}><Icon name="bell"/></button><div className="time"><Icon name="clock" size={15}/>{now}</div></div>
    </header>

    <section className="dashboard-grid">
      <div className="page-heading">
        <div><span className="overline">PATIENT OVERVIEW</span><h1>Live monitoring</h1><p>Non-invasive sensor fusion with on-device TinyML analysis</p></div>
        <div className="last-sync"><span>LAST SYNC</span><strong>Just now</strong></div>
      </div>

      <section className="vitals">
        <VitalCard type="pulse" label="Heart rate" value={heart} unit="BPM" sensor="APDS9960" tone="#ff6680"><Sparkline path={heartPath} color="#ff6680" fillId="heart-fill"/></VitalCard>
        <VitalCard type="lungs" label="Respiratory rate" value={respiration} unit="BREATHS / MIN" sensor="BMP180" tone="#29d6c4"><Sparkline path={breathPath} color="#29d6c4" fillId="breath-fill"/></VitalCard>
        <VitalCard type="motion" label="Mobility index" value={mobility} unit="% OF BASELINE" sensor="MPU6050" tone="#a78bfa"><div className="bar-chart">{motionBars.map((height, index) => <i key={index} style={{ height: `${height}%` }}/>)}</div></VitalCard>
      </section>

      <aside className="risk-card">
        <div className="risk-heading"><span>SEPSIS PROXY SCORE</span><Icon name="shield" size={19}/></div>
        <div className="gauge"><svg viewBox="0 0 160 160"><circle cx="80" cy="80" r="64"/><circle className="gauge-value" cx="80" cy="80" r="64"/></svg><div><strong>1</strong><span>/ 3</span></div></div>
        <div className="risk-level"><i/>MONITOR CLOSELY</div>
        <p>One physiological signal is currently outside its normal range.</p>
        <div className="risk-factors"><div><span>Elevated pulse</span><strong>+1</strong></div><div><span>Rapid breathing</span><strong>0</strong></div><div><span>Reduced mobility</span><strong>0</strong></div></div>
        <div className="disclaimer"><Icon name="shield" size={14}/>Prototype support tool — not a medical diagnosis</div>
      </aside>

      <section className="trend-card">
        <div className="panel-heading"><div><span className="overline">COMBINED TREND</span><h2>Physiological signals</h2></div><div className="ranges">{["1H", "6H", "12H", "24H"].map(item => <button className={range === item ? "active" : ""} onClick={() => setRange(item)} key={item}>{item}</button>)}</div></div>
        <div className="legend"><span><i className="heart-dot"/>Heart rate</span><span><i className="breath-dot"/>Respiration</span><span><i className="move-dot"/>Mobility</span><span className="threshold">··· Alert threshold</span></div>
        <div className="large-chart">
          <div className="y-labels"><span>120</span><span>100</span><span>80</span><span>60</span></div>
          <div className="plot"><div className="alert-line"><span>THRESHOLD</span></div><svg viewBox="0 0 800 210" preserveAspectRatio="none"><path className="area" d="M0 145 C50 137 82 141 120 132 S190 117 230 126 302 147 350 126 422 108 462 115 530 131 575 108 635 97 680 111 746 83 800 89 L800 210 0 210Z"/><path className="line heart-line" d="M0 145 C50 137 82 141 120 132 S190 117 230 126 302 147 350 126 422 108 462 115 530 131 575 108 635 97 680 111 746 83 800 89"/><path className="line breath-line" d="M0 171 C58 165 91 173 140 164 S226 167 275 158 353 161 410 151 500 156 548 146 640 153 697 142 758 146 800 137"/><path className="line move-line" d="M0 78 C65 83 99 72 153 79 S253 87 303 82 386 73 445 80 542 91 590 84 677 75 723 80 772 72 800 76"/></svg><div className="x-labels"><span>09:00</span><span>09:15</span><span>09:30</span><span>09:45</span><span>10:00</span></div></div>
        </div>
      </section>

      <aside className="ai-card">
        <div className="ai-title"><span className="ai-icon"><Icon name="brain" size={23}/></span><div><span>TINYML INSIGHT</span><strong>Pattern analysis</strong></div><span className="confidence">87% confidence</span></div>
        <div className="ai-result"><span className="analysis-ring"><i/><i/><Icon name="brain" size={25}/></span><div><strong>Mild deviation detected</strong><p>Pulse is trending above the established baseline. Other signals remain stable.</p></div></div>
        <div className="model-stats"><div><span>MODEL</span><strong>SENTRA Edge v1.2</strong></div><div><span>INFERENCE</span><strong>18 ms</strong></div><div><span>LAST RUN</span><strong>2 sec ago</strong></div></div>
        <div className="recommendation"><span>RECOMMENDED ACTION</span><p>Continue monitoring. Reassess if pulse remains elevated for more than 15 minutes.</p></div>
      </aside>

      <section className="device-strip">
        <div><span className="device-icon"><Icon name="chip"/></span><p><strong>MYOSA board</strong><span>Processing normally</span></p><b><i/>ACTIVE</b></div>
        <div><span className="device-icon"><Icon name="wifi"/></span><p><strong>I²C sensor bus</strong><span>4 devices connected</span></p><b><i/>STABLE</b></div>
        <div><span className="device-icon"><Icon name="battery"/></span><p><strong>Power status</strong><span>Estimated 7h 24m</span></p><b>86%</b></div>
        <div className="sampling"><span>SAMPLING RATE</span><strong>25 Hz</strong></div>
      </section>
    </section>
  </main>;
}

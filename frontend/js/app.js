/* Smart Study Group Formation System - frontend (vanilla JS). All scoring/grouping comes from the C++ API. */
const $ = (s, r = document) => r.querySelector(s);
const $$ = (s, r = document) => [...r.querySelectorAll(s)];
const esc = s => String(s ?? '').replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
const P = (d) => `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">${d}</svg>`;
const IC = {
  logo: P('<circle cx="9" cy="8" r="3.2"/><circle cx="17" cy="9" r="2.6"/><path d="M3 20c0-3.5 2.7-5.5 6-5.5s6 2 6 5.5M15 15c3 0 6 1.5 6 4.5"/>'),
  dash: P('<rect x="3" y="3" width="7" height="9" rx="1.5"/><rect x="14" y="3" width="7" height="5" rx="1.5"/><rect x="14" y="12" width="7" height="9" rx="1.5"/><rect x="3" y="16" width="7" height="5" rx="1.5"/>'),
  add: P('<circle cx="10" cy="8" r="4"/><path d="M2 21c0-4 3.5-6 8-6M19 14v6M16 17h6"/>'),
  dir: P('<path d="M16 21v-2a4 4 0 0 0-4-4H6a4 4 0 0 0-4 4v2"/><circle cx="9" cy="7" r="4"/><path d="M22 21v-2a4 4 0 0 0-3-3.9M16 3.1a4 4 0 0 1 0 7.8"/>'),
  find: P('<circle cx="11" cy="11" r="7"/><path d="M21 21l-4.3-4.3"/>'),
  groups: P('<circle cx="12" cy="5" r="2.5"/><circle cx="5" cy="18" r="2.5"/><circle cx="19" cy="18" r="2.5"/><path d="M12 8v4M12 12l-5.5 4M12 12l5.5 4"/>'),
  algo: P('<path d="M4 20V10M10 20V4M16 20v-8M22 20H2"/>'),
  info: P('<circle cx="12" cy="12" r="9"/><path d="M12 16v-5M12 8h.01"/>'),
  menu: P('<path d="M3 6h18M3 12h18M3 18h18"/>'),
  user: P('<circle cx="12" cy="8" r="4"/><path d="M4 21c0-4 3.5-6 8-6s8 2 8 6"/>'),
  check: P('<path d="M5 12l5 5L20 7"/>'),
  star: P('<path d="M12 3l2.7 5.6 6.1.9-4.4 4.3 1 6.1L12 17l-5.4 2.9 1-6.1L3.2 9.5l6.1-.9z"/>'),
  edit: P('<path d="M12 20h9M16.5 3.5a2.1 2.1 0 0 1 3 3L7 19l-4 1 1-4z"/>'),
  trash: P('<path d="M3 6h18M8 6V4h8v2M19 6l-1 14H6L5 6M10 11v6M14 11v6"/>'),
  eye: P('<path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8S1 12 1 12z"/><circle cx="12" cy="12" r="3"/>'),
  bolt: P('<path d="M13 2L4 14h7l-1 8 9-12h-7z"/>'),
  clock: P('<circle cx="12" cy="12" r="9"/><path d="M12 7v5l3 2"/>'),
  x: P('<path d="M18 6L6 18M6 6l12 12"/>'),
  scale: P('<path d="M12 3v18M5 7h14M5 7l-3 7a4 4 0 0 0 6 0zM19 7l-3 7a4 4 0 0 0 6 0z"/>')
};
const SUBJECTS = ['DSA', 'DBMS', 'Mathematics', 'Operating Systems', 'Computer Networks', 'Web Technologies', 'Python Programming', 'Software Engineering', 'OOP with C++', 'Theory of Computation'];
const GOALS = ['Exam Preparation', 'Concept Clarity', 'Placement Preparation', 'Project Work', 'Competitive Programming'];
const DAYS = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];
const DEPTS = ['Computer Engineering', 'Information Technology', 'Electronics & Telecom', 'AI & Data Science', 'Mechanical Engineering'];
const COLORS = ['#2563eb', '#7c3aed', '#059669', '#d97706', '#db2777', '#0891b2', '#4f46e5', '#dc2626'];
const LV = { Beginner: '#f59e0b', Intermediate: '#2563eb', Advanced: '#10b981' };
const hr = h => { h = +h; const hh = h % 24, h12 = hh % 12 || 12; return h12 + (hh >= 12 && h !== 24 ? ' PM' : ' AM'); };
const initials = n => n.split(/\s+/).map(w => w[0]).slice(0, 2).join('').toUpperCase();
const color = id => COLORS[id % COLORS.length];
const avatar = s => `<div class="avatar" style="background:linear-gradient(135deg,${color(s.id)},${color(s.id + 3)})">${esc(initials(s.name))}</div>`;

async function api(method, url, body) {
  const r = await fetch(url, { method, headers: { 'Content-Type': 'application/json' }, body: body ? JSON.stringify(body) : undefined });
  const d = await r.json().catch(() => ({}));
  if (!r.ok) throw new Error(d.error || 'Request failed');
  return d;
}
function toast(msg, kind = 'ok') {
  const t = document.createElement('div'); t.className = 'toast ' + kind; t.textContent = msg;
  $('#toasts').appendChild(t); setTimeout(() => t.remove(), 3600);
}
function modal(html) { $('#modalBox').innerHTML = html; $('#modal').classList.add('show'); }
function closeModal() { $('#modal').classList.remove('show'); }
$('#modal').addEventListener('click', e => { if (e.target.id === 'modal') closeModal(); });
function confirmBox(title, text, onYes) {
  modal(`<h2>${esc(title)}</h2><p style="color:var(--muted)">${esc(text)}</p><div class="actions"><button class="btn btn-ghost" id="cNo">Cancel</button><button class="btn btn-primary" id="cYes" style="background:#dc2626">Yes, continue</button></div>`);
  $('#cNo').onclick = closeModal; $('#cYes').onclick = () => { closeModal(); onYes(); };
}
const ring = (pct, size = 96, stroke = 9, col = '#2563eb', track = '#e3e9f4') => {
  const r = (size - stroke) / 2, c = 2 * Math.PI * r;
  return `<div class="ring" style="width:${size}px;height:${size}px"><svg width="${size}" height="${size}" style="transform:rotate(-90deg)"><circle cx="${size / 2}" cy="${size / 2}" r="${r}" fill="none" stroke="${track}" stroke-width="${stroke}"/><circle cx="${size / 2}" cy="${size / 2}" r="${r}" fill="none" stroke="${col}" stroke-width="${stroke}" stroke-linecap="round" stroke-dasharray="${c * Math.min(pct, 100) / 100} ${c}"/></svg><b>${Math.round(pct)}%</b></div>`;
};
const animateBars = root => requestAnimationFrame(() => requestAnimationFrame(() => $$('[data-w]', root).forEach(b => b.style.width = b.dataset.w + '%')));
const bar = (pct, cls = '') => `<div class="bar ${cls}"><i data-w="${Math.max(0, Math.min(100, pct))}"></i></div>`;
const tags = (a, hl = []) => `<div class="mini-tags">${a.map(x => `<span class="${hl.includes(x) ? 'hl' : ''}">${esc(x)}</span>`).join('')}</div>`;

/* ---------------- state + router ---------------- */
let students = [], editing = null, groupsData = null;
const gen = { algorithm: 'greedy', threshold: 50, maxSize: 4, selected: null };
const PAGES = [
  ['dashboard', 'Dashboard', IC.dash, 'Overview of students, groups and matching statistics'],
  ['register', 'Register Student', IC.add, 'Add a student profile for study-partner matching'],
  ['directory', 'Student Directory', IC.dir, 'Search, filter, view, edit and delete student records'],
  ['find', 'Find Study Groups', IC.find, 'Compatibility scores, graph and algorithm comparison'],
  ['groups', 'Generated Groups', IC.groups, 'Recommended study groups with explanations'],
  ['analysis', 'Algorithm Analysis', IC.algo, 'The DSA concepts behind the project'],
  ['about', 'About Project', IC.info, 'Project details, formula and API']
];
$('#brandLogo').innerHTML = IC.logo; $('#menuBtn').innerHTML = IC.menu;
$('#topRegister').innerHTML = IC.add + ' New Student';
$('#nav').innerHTML = PAGES.map(p => `<a data-page="${p[0]}" href="#${p[0]}">${p[2]}<span>${p[1]}</span></a>`).join('');
$('#topRegister').onclick = () => { editing = null; location.hash = 'register'; if (current === 'register') renderRegister(); };
$('#menuBtn').onclick = () => { $('#sidebar').classList.toggle('open'); $('#overlay').classList.toggle('show'); };
$('#overlay').onclick = () => { $('#sidebar').classList.remove('open'); $('#overlay').classList.remove('show'); };
let current = '';
const R = { dashboard: renderDashboard, register: renderRegister, directory: renderDirectory, find: renderFind, groups: renderGroups, analysis: renderAnalysis, about: renderAbout };
async function route() {
  const id = (location.hash || '#dashboard').slice(1); const pg = PAGES.find(p => p[0] === id) || PAGES[0];
  current = pg[0];
  $$('.page').forEach(p => p.classList.toggle('active', p.id === 'page-' + current));
  $$('#nav a').forEach(a => a.classList.toggle('active', a.dataset.page === current));
  $('#pageTitle').textContent = pg[1]; $('#pageSub').textContent = pg[3];
  $('#sidebar').classList.remove('open'); $('#overlay').classList.remove('show');
  try { students = await api('GET', '/api/students'); await R[current](); } catch (e) { toast(e.message, 'err'); }
  window.scrollTo(0, 0);
}
window.addEventListener('hashchange', route);

/* ---------------- dashboard ---------------- */
async function renderDashboard() {
  const s = await api('GET', '/api/stats'), el = $('#page-dashboard');
  const maxC = Math.max(1, ...s.popularSubjects.map(x => x.count));
  const lv = s.levels, tot = Math.max(1, lv.Beginner + lv.Intermediate + lv.Advanced);
  let off = 0; const C = 2 * Math.PI * 40;
  const donut = ['Beginner', 'Intermediate', 'Advanced'].map(k => { const len = C * lv[k] / tot, o = off; off += len; return `<circle cx="50" cy="50" r="40" fill="none" stroke="${LV[k]}" stroke-width="16" stroke-dasharray="${len} ${C - len}" stroke-dashoffset="${-o}"/>`; }).join('');
  el.innerHTML = `
  <div class="banner" style="margin-bottom:20px"><div><h2>Find the right study partners, automatically</h2><p>Compatibility is calculated in C++ from subjects (40%), timings (30%), goals (20%) and skills (10%), then grouped with Greedy or BFS algorithms.</p></div>
  <button class="btn btn-primary" style="background:#fff;color:#1d4ed8" id="dGen">${IC.bolt} Generate Groups</button></div>
  <div class="grid g4">
    <div class="card stat"><div class="ico i-blue">${IC.dir}</div><div><div class="num">${s.totalStudents}</div><div class="lbl">Registered Students</div></div></div>
    <div class="card stat"><div class="ico i-navy">${IC.groups}</div><div><div class="num">${s.totalGroups}</div><div class="lbl">Study Groups Generated</div></div></div>
    <div class="card stat"><div class="ico i-green">${IC.check}</div><div><div class="num">${s.successfulMatches}</div><div class="lbl">Successful Matches</div></div></div>
    <div class="card stat"><div class="ico i-violet">${IC.star}</div><div><div class="num">${s.avgCompatibility}%</div><div class="lbl">Avg Group Compatibility</div></div></div>
  </div>
  <div class="grid g-main mt">
    <div class="card"><div class="card-head"><div><h3>Most Popular Subjects</h3><div class="sub" style="margin:0">Number of students studying each subject</div></div></div>
      ${s.popularSubjects.length ? s.popularSubjects.map(x => `<div class="hbar"><span>${esc(x.subject)}</span>${bar(x.count / maxC * 100)}<span class="n">${x.count}</span></div>`).join('') : '<div class="empty">No data yet</div>'}</div>
    <div class="card"><h3>Experience Levels</h3><div class="sub">Distribution of registered students</div>
      <div class="donut-wrap"><svg width="150" height="150" viewBox="0 0 100 100" style="transform:rotate(-90deg)">${donut}</svg>
      <div class="legend">${['Beginner', 'Intermediate', 'Advanced'].map(k => `<div><i style="background:${LV[k]}"></i>${k}: <b>${lv[k]}</b></div>`).join('')}</div></div></div>
  </div>
  <div class="grid g-main mt">
    <div class="card"><div class="card-head"><div><h3>Recent Registrations</h3><div class="sub" style="margin:0">Latest students added to the system</div></div><a class="btn btn-ghost btn-sm" href="#directory">View all</a></div>
      <div class="recent">${s.recent.map(r => `<div class="item" data-view="${r.id}">${avatar(r)}<div class="meta"><b>${esc(r.name)}</b><span>${esc(r.department)} · Sem ${r.semester} · ${esc(r.subjects.slice(0, 3).join(', '))}</span></div><span class="badge b-${r.level}">${r.level}</span></div>`).join('') || '<div class="empty">No students yet</div>'}</div></div>
    <div class="card"><h3>Group Formation Statistics</h3><div class="sub">${s.algorithm ? 'Latest run: ' + (s.algorithm === 'greedy' ? 'Greedy' : 'BFS') + ' · ' + esc(s.generatedAt) : 'No groups generated yet'}</div>
      <div class="donut-wrap" style="margin-bottom:12px">${ring(s.matchRate, 110, 11, '#10b981')}<div><b style="font-size:15px">Match rate</b><div style="color:var(--muted);font-size:13px">${s.successfulMatches} of ${s.totalStudents} students placed in a group</div></div></div>
      <div class="kv"><span>Groups formed</span><b>${s.totalGroups}</b></div><div class="kv"><span>Unassigned students</span><b>${s.unassigned}</b></div>
      <div class="kv"><span>Subject coverage</span><b>${s.coveragePct}%</b></div><div class="kv"><span>Average compatibility</span><b>${s.avgCompatibility}%</b></div></div>
  </div>`;
  animateBars(el);
  $('#dGen').onclick = async () => { await generate(); location.hash = 'groups'; };
  $$('[data-view]', el).forEach(i => i.onclick = () => viewProfile(+i.dataset.view));
}

/* ---------------- register / edit ---------------- */
let form = {};
function renderRegister() {
  const e = editing ? students.find(s => s.id === editing) : null;
  if (editing && !e) editing = null;
  form = e ? JSON.parse(JSON.stringify(e)) : { subjects: [], days: [], goals: [], skills: [], startHour: 16, endHour: 18, level: 'Beginner', groupSize: 4, semester: 3, department: DEPTS[0] };
  const customSub = form.subjects.filter(x => !SUBJECTS.includes(x));
  const hours = (sel) => Array.from({ length: 24 }, (_, h) => `<option value="${h + 1 - (sel === 'start' ? 1 : 0)}">${hr(h + 1 - (sel === 'start' ? 1 : 0))}</option>`).join('');
  $('#page-register').innerHTML = `<div class="card" style="max-width:980px"><div class="card-head"><div><h3>${e ? 'Edit Student – ' + esc(e.name) : 'Student Registration'}</h3><div class="sub" style="margin:0">Fill in the profile used for compatibility matching.</div></div></div>
  <div class="form-section">Basic information</div>
  <div class="form-grid">
    <div class="field"><label>Full name *</label><input type="text" id="fName" placeholder="e.g. Aarav Sharma" value="${esc(form.name || '')}"></div>
    <div class="field"><label>Student ID *</label><input type="text" id="fSid" placeholder="e.g. 2024CE001" value="${esc(form.studentId || '')}"></div>
    <div class="field"><label>Department *</label><select id="fDept">${[...new Set([...DEPTS, form.department])].map(d => `<option ${d === form.department ? 'selected' : ''}>${esc(d)}</option>`).join('')}</select></div>
    <div class="field"><label>Semester *</label><select id="fSem">${[1, 2, 3, 4, 5, 6, 7, 8].map(n => `<option ${n === +form.semester ? 'selected' : ''}>${n}</option>`).join('')}</select></div>
  </div>
  <div class="form-section">Subjects & goals</div>
  <div class="field"><label>Subjects you want to study *</label><div class="chips" id="cSub"></div>
    <div class="add-row"><input type="text" id="subIn" placeholder="Add another subject…"><button class="btn btn-ghost btn-sm" id="subAdd" type="button">Add</button></div></div>
  <div class="field"><label>Learning goals</label><div class="chips" id="cGoal"></div></div>
  <div class="form-section">Availability</div>
  <div class="field"><label>Preferred study days *</label><div class="chips" id="cDays"></div></div>
  <div class="field"><label>Preferred study timing *</label><div class="time-row"><select id="fStart">${hours('start')}</select><span>to</span><select id="fEnd">${hours('end')}</select></div></div>
  <div class="form-section">Skills & experience</div>
  <div class="field"><label>Experience level *</label><div class="seg" id="segLv">${['Beginner', 'Intermediate', 'Advanced'].map(l => `<button type="button" data-l="${l}" class="${l === form.level ? 'on' : ''}">${l}</button>`).join('')}</div></div>
  <div class="field"><label>Skills you can offer</label><div class="tag-box" id="skBox"><input type="text" id="skIn" placeholder="Type a skill and press Enter (e.g. C++)"></div></div>
  <div class="field" style="max-width:260px"><label>Preferred group size</label><select id="fGs">${[2, 3, 4, 5, 6].map(n => `<option ${n === +form.groupSize ? 'selected' : ''}>${n}</option>`).join('')}</select></div>
  <div class="form-actions"><button class="btn btn-primary" id="fSave">${IC.check} ${e ? 'Save Changes' : 'Register Student'}</button><button class="btn btn-ghost" id="fCancel" type="button">${e ? 'Cancel' : 'Reset'}</button></div></div>`;
  $('#fStart').value = form.startHour; $('#fEnd').value = form.endHour;
  const drawChips = () => {
    const mk = (id, list, key) => { $('#' + id).innerHTML = list.map(x => `<div class="chip ${form[key].includes(x) ? 'on' : ''}" data-v="${esc(x)}">${esc(x)}</div>`).join(''); $$('#' + id + ' .chip').forEach(c => c.onclick = () => { const v = c.dataset.v, i = form[key].indexOf(v); i >= 0 ? form[key].splice(i, 1) : form[key].push(v); drawChips(); }); };
    mk('cSub', [...SUBJECTS, ...form.subjects.filter(x => !SUBJECTS.includes(x))], 'subjects'); mk('cGoal', GOALS, 'goals'); mk('cDays', DAYS, 'days');
    $$('.tag', $('#skBox')).forEach(t => t.remove());
    form.skills.forEach(sk => { const t = document.createElement('span'); t.className = 'tag'; t.innerHTML = `${esc(sk)} <button type="button">×</button>`; t.querySelector('button').onclick = () => { form.skills = form.skills.filter(x => x !== sk); drawChips(); }; $('#skBox').insertBefore(t, $('#skIn')); });
  };
  drawChips();
  const addSub = () => { const v = $('#subIn').value.trim(); if (v && !form.subjects.some(x => x.toLowerCase() === v.toLowerCase())) form.subjects.push(v); $('#subIn').value = ''; drawChips(); };
  $('#subAdd').onclick = addSub; $('#subIn').onkeydown = ev => { if (ev.key === 'Enter') { ev.preventDefault(); addSub(); } };
  $('#skIn').onkeydown = ev => { if (ev.key === 'Enter' || ev.key === ',') { ev.preventDefault(); const v = $('#skIn').value.replace(',', '').trim(); if (v && !form.skills.includes(v)) form.skills.push(v); $('#skIn').value = ''; drawChips(); } };
  $$('#segLv button').forEach(b => b.onclick = () => { form.level = b.dataset.l; $$('#segLv button').forEach(x => x.classList.toggle('on', x === b)); });
  $('#fCancel').onclick = () => { if (editing) { editing = null; location.hash = 'directory'; } else renderRegister(); };
  $('#fSave').onclick = async () => {
    const pending = $('#skIn').value.trim(); if (pending && !form.skills.includes(pending)) form.skills.push(pending);
    const body = { name: $('#fName').value, studentId: $('#fSid').value, department: $('#fDept').value, semester: +$('#fSem').value, subjects: form.subjects, days: form.days, goals: form.goals, skills: form.skills, startHour: +$('#fStart').value, endHour: +$('#fEnd').value, level: form.level, groupSize: +$('#fGs').value };
    try {
      if (editing) { await api('PUT', '/api/students/' + editing, body); toast('Student updated'); editing = null; location.hash = 'directory'; }
      else { await api('POST', '/api/students', body); toast('Student registered successfully'); location.hash = 'directory'; }
    } catch (err) { toast(err.message, 'err'); }
  };
}

/* ---------------- directory ---------------- */
const filt = { q: '', subject: '', level: '' };
function renderDirectory() {
  const subs = [...new Set(students.flatMap(s => s.subjects))].sort();
  $('#page-directory').innerHTML = `<div class="card"><div class="toolbar">
    <div class="search">${IC.find}<input type="search" id="dq" placeholder="Search students by name…" value="${esc(filt.q)}"></div>
    <select id="dsub"><option value="">All subjects</option>${subs.map(x => `<option ${x === filt.subject ? 'selected' : ''}>${esc(x)}</option>`).join('')}</select>
    <select id="dlv"><option value="">All levels</option>${['Beginner', 'Intermediate', 'Advanced'].map(x => `<option ${x === filt.level ? 'selected' : ''}>${x}</option>`).join('')}</select>
    <button class="btn btn-primary" id="dnew">${IC.add} Register Student</button></div>
    <div id="dtable"></div></div>`;
  const draw = () => {
    const list = students.filter(s => s.name.toLowerCase().includes(filt.q.toLowerCase()) && (!filt.subject || s.subjects.includes(filt.subject)) && (!filt.level || s.level === filt.level));
    $('#dtable').innerHTML = list.length ? `<div class="table-wrap"><table><thead><tr><th>Student</th><th>Department</th><th>Subjects</th><th>Availability</th><th>Level</th><th>Group</th><th>Actions</th></tr></thead><tbody>
      ${list.map(s => `<tr><td><div class="person">${avatar(s)}<div><b>${esc(s.name)}</b><span>${esc(s.studentId)}</span></div></div></td><td>${esc(s.department)}<br><span style="color:var(--muted);font-size:12.5px">Semester ${s.semester}</span></td><td>${tags(s.subjects)}</td><td>${esc(s.days.join(', '))}<br><span style="color:var(--muted);font-size:12.5px">${hr(s.startHour)} – ${hr(s.endHour)}</span></td><td><span class="badge b-${s.level}">${s.level}</span></td><td>${groupOf(s.id)}</td>
      <td><div class="row-actions"><button class="icon-btn" title="View" data-a="v" data-id="${s.id}">${IC.eye}</button><button class="icon-btn" title="Edit" data-a="e" data-id="${s.id}">${IC.edit}</button><button class="icon-btn del" title="Delete" data-a="d" data-id="${s.id}">${IC.trash}</button></div></td></tr>`).join('')}</tbody></table></div>
      <div style="margin-top:12px;color:var(--muted);font-size:13px">Showing ${list.length} of ${students.length} students</div>` : `<div class="empty"><div class="big">${IC.dir}</div><h3>No students found</h3><p>Try changing the search or filters.</p></div>`;
    $$('#dtable [data-a]').forEach(b => b.onclick = () => {
      const id = +b.dataset.id, s = students.find(x => x.id === id);
      if (b.dataset.a === 'v') viewProfile(id);
      if (b.dataset.a === 'e') { editing = id; location.hash = 'register'; if (current === 'register') renderRegister(); }
      if (b.dataset.a === 'd') confirmBox('Delete student?', `${s.name} will be permanently removed (and removed from any group).`, async () => { try { await api('DELETE', '/api/students/' + id); toast('Student deleted'); students = await api('GET', '/api/students'); await refreshGroupsMap(); draw(); } catch (e) { toast(e.message, 'err'); } });
    });
  };
  $('#dq').oninput = e => { filt.q = e.target.value; draw(); };
  $('#dsub').onchange = e => { filt.subject = e.target.value; draw(); };
  $('#dlv').onchange = e => { filt.level = e.target.value; draw(); };
  $('#dnew').onclick = () => { editing = null; location.hash = 'register'; };
  refreshGroupsMap().then(draw);
}
let groupMap = {};
async function refreshGroupsMap() { groupsData = await api('GET', '/api/groups'); groupMap = {}; groupsData.groups.forEach(g => g.members.forEach(m => groupMap[m.id] = g.number)); }
const groupOf = id => groupMap[id] ? `<span class="pill ok">Group ${groupMap[id]}</span>` : '<span style="color:var(--muted)">—</span>';
function viewProfile(id) {
  const s = students.find(x => x.id === id); if (!s) return;
  modal(`<div class="profile-head">${avatar(s)}<div><h2 style="margin:0">${esc(s.name)}</h2><span style="color:var(--muted)">${esc(s.studentId)} · ${esc(s.department)} · Semester ${s.semester}</span></div></div>
  <div class="prow"><span>Experience level</span><div><span class="badge b-${s.level}">${s.level}</span></div></div>
  <div class="prow"><span>Subjects</span>${tags(s.subjects)}</div><div class="prow"><span>Study days</span><div>${esc(s.days.join(', '))}</div></div>
  <div class="prow"><span>Study timing</span><div>${hr(s.startHour)} – ${hr(s.endHour)}</div></div><div class="prow"><span>Learning goals</span>${s.goals.length ? tags(s.goals) : '—'}</div>
  <div class="prow"><span>Skills offered</span>${s.skills.length ? tags(s.skills) : '—'}</div><div class="prow"><span>Preferred group size</span><div>${s.groupSize} students</div></div>
  <div class="actions"><button class="btn btn-ghost" id="mClose">Close</button><button class="btn btn-primary" id="mFind">${IC.find} Find partners</button></div>`);
  $('#mClose').onclick = closeModal; $('#mFind').onclick = () => { closeModal(); gen.selected = id; location.hash = 'find'; if (current === 'find') renderFind(); };
}

/* ---------------- find study groups ---------------- */
async function generate() {
  try {
    groupsData = await api('POST', '/api/groups/generate', { algorithm: gen.algorithm, threshold: gen.threshold, maxSize: gen.maxSize });
    toast(`${groupsData.groups.length} groups generated using ${gen.algorithm === 'greedy' ? 'Greedy' : 'BFS'}`); return true;
  } catch (e) { toast(e.message, 'err'); return false; }
}
async function renderFind() {
  const el = $('#page-find');
  if (!gen.selected || !students.find(s => s.id === gen.selected)) gen.selected = students[0]?.id;
  el.innerHTML = `<div class="card"><h3>Group Generation</h3><div class="sub">Choose an algorithm and parameters. Pairs below the threshold are not connected in the graph.</div>
    <div class="controls">
      <div class="field"><label>Algorithm</label><select id="gAlgo"><option value="greedy">Greedy (best pairs first + DSU)</option><option value="bfs">BFS (connected components)</option></select></div>
      <div class="field"><label>Compatibility threshold: <b id="thv">${gen.threshold}</b>%</label><input type="range" id="gTh" min="30" max="90" step="5" value="${gen.threshold}"></div>
      <div class="field"><label>Maximum group size</label><select id="gMax">${[2, 3, 4, 5, 6].map(n => `<option ${n === gen.maxSize ? 'selected' : ''}>${n}</option>`).join('')}</select></div>
      <div style="display:flex;gap:10px"><button class="btn btn-primary" id="gGo">${IC.bolt} Generate</button><button class="btn btn-dark" id="gCmp">${IC.scale} Compare</button></div></div>
    <div id="cmpBox"></div></div>
  <div class="grid g2 mt" style="grid-template-columns:1.15fr 1fr">
    <div class="card"><div class="card-head"><div><h3>Find Study Partners</h3><div class="sub" style="margin:0">Compatibility of one student with everyone else</div></div></div>
      <div class="field"><select id="gSel">${students.map(s => `<option value="${s.id}" ${s.id === gen.selected ? 'selected' : ''}>${esc(s.name)} (${esc(s.studentId)})</option>`).join('')}</select></div><div id="matches"></div></div>
    <div class="card"><h3>Compatibility Graph</h3><div class="sub">Vertices = students · edges = compatible pairs (weight = score). Click a student to see their matches.</div><div id="graphBox"></div><div id="graphInfo"></div></div></div>`;
  $('#gAlgo').value = gen.algorithm;
  $('#gAlgo').onchange = e => gen.algorithm = e.target.value; $('#gMax').onchange = e => gen.maxSize = +e.target.value;
  let tm; $('#gTh').oninput = e => { gen.threshold = +e.target.value; $('#thv').textContent = gen.threshold; clearTimeout(tm); tm = setTimeout(drawGraph, 150); };
  $('#gSel').onchange = e => { gen.selected = +e.target.value; drawMatches(); };
  $('#gGo').onclick = async () => { $('#gGo').disabled = true; if (await generate()) location.hash = 'groups'; $('#gGo').disabled = false; };
  $('#gCmp').onclick = compare;
  drawMatches(); drawGraph();
}
async function drawMatches() {
  const box = $('#matches'); if (!gen.selected) { box.innerHTML = '<div class="empty">Register students first.</div>'; return; }
  const d = await api('GET', '/api/compatibility/' + gen.selected);
  box.innerHTML = d.matches.map(m => `<div class="match ${m.meaningful ? '' : 'weak'}"><div class="match-top">${avatar(m.student)}<div><b>${esc(m.student.name)}</b><div><span class="badge b-${m.student.level}">${m.student.level}</span> <span class="pill ${m.meaningful ? 'ok' : 'no'}">${m.meaningful ? 'Compatible' : 'No shared subject/time'}</span></div></div><div class="score"><b>${Math.round(m.total)}%</b><small>compatibility</small></div></div>
    <div class="parts">${[['Subjects', m.subjectScore, 40, ''], ['Timings', m.timingScore, 30, 'green'], ['Goals', m.goalScore, 20, 'violet'], ['Skills', m.skillScore, 10, 'amber']].map(p => `<div class="part"><small><span>${p[0]}</span><span>${(+p[1]).toFixed(1)}/${p[2]}</span></small>${bar(p[1] / p[2] * 100, p[3])}</div>`).join('')}</div>
    <div class="why">${m.commonSubjects.length ? '<b>Common subjects:</b> ' + esc(m.commonSubjects.join(', ')) + ' · ' : ''}${m.overlap !== 'none' ? '<b>Overlap:</b> ' + esc(m.commonDays.join(', ')) + ' ' + esc(m.overlap) : 'No overlapping study time'}${m.commonGoals.length ? ' · <b>Goals:</b> ' + esc(m.commonGoals.join(', ')) : ''}</div></div>`).join('') || '<div class="empty">Need at least two students.</div>';
  animateBars(box);
}
async function drawGraph() {
  const g = await api('GET', '/api/graph?threshold=' + gen.threshold), box = $('#graphBox'); if (!box) return;
  const W = 520, H = 460, cx = W / 2, cy = H / 2, R0 = Math.min(W, H) / 2 - 62, n = g.nodes.length, pos = {};
  g.nodes.forEach((nd, i) => { const a = -Math.PI / 2 + 2 * Math.PI * i / Math.max(n, 1); pos[nd.id] = [cx + R0 * Math.cos(a), cy + R0 * Math.sin(a)]; });
  const sel = gen.selected;
  box.innerHTML = `<svg viewBox="0 0 ${W} ${H}">${g.edges.map(e => { const [x1, y1] = pos[e.from], [x2, y2] = pos[e.to], hot = e.from === sel || e.to === sel; return `<line class="gedge" x1="${x1}" y1="${y1}" x2="${x2}" y2="${y2}" stroke-width="${1 + e.weight / 22}" opacity="${hot ? .95 : .18 + e.weight / 200}" ${hot ? 'stroke="#0b1b3b"' : ''}><title>${esc(g.nodes.find(x => x.id === e.from).name)} ↔ ${esc(g.nodes.find(x => x.id === e.to).name)}: ${e.weight}%</title></line>`; }).join('')}
  ${g.nodes.map(nd => { const [x, y] = pos[nd.id], ty = y < cy ? y - 28 : y + 36; return `<g class="gnode" data-id="${nd.id}"><circle cx="${x}" cy="${y}" r="${nd.id === sel ? 24 : 20}" fill="${LV[nd.level]}" stroke="#fff" stroke-width="3"/><text x="${x}" y="${y + 4}" text-anchor="middle" fill="#fff" font-size="11" font-weight="700" pointer-events="none">${esc(initials(nd.name))}</text><text class="gtext" x="${x}" y="${ty}" text-anchor="middle">${esc(nd.name.split(' ')[0])}</text></g>`; }).join('')}</svg>`;
  $$('.gnode', box).forEach(nd => nd.onclick = () => { gen.selected = +nd.dataset.id; $('#gSel').value = gen.selected; drawMatches(); drawGraph(); });
  $('#graphInfo').innerHTML = `<div class="glegend">${Object.entries(LV).map(([k, c]) => `<span><i style="background:${c}"></i>${k}</span>`).join('')}</div>
  <div class="glegend"><span><b>${g.edgeCount}</b> edges</span><span>Components (BFS): <b>${g.bfsComponents}</b></span><span>Components (DSU): <b>${g.dsuComponents}</b></span></div>
  <details style="margin-top:10px"><summary style="cursor:pointer;font-weight:600;color:var(--blue2)">Show adjacency list</summary><div class="adj">${g.adjacency.map(a => `<b>${esc(a.name)}</b> → ${a.neighbors.length ? a.neighbors.map(x => `${esc(x.name.split(' ')[0])}<em>(${x.weight})</em>`).join(', ') : '<em>no edges</em>'}`).join('<br>')}</div></details>`;
}
async function compare() {
  try {
    const d = await api('POST', '/api/groups/compare', { threshold: gen.threshold, maxSize: gen.maxSize }), a = d.greedy.metrics, b = d.bfs.metrics;
    const row = (l, x, y, hi = true, fmt = v => v) => { const w = x === y ? 0 : ((x > y) === hi ? 1 : 2); return `<tr><th>${l}</th><td class="${w === 1 ? 'win' : ''}"><b>${fmt(x)}</b></td><td class="${w === 2 ? 'win' : ''}"><b>${fmt(y)}</b></td></tr>`; };
    const list = x => x.groups.map(g => `<div style="margin:3px 0;font-size:12.5px">• ${g.members.map(m => esc(m.name.split(' ')[0])).join(', ')} <span style="color:var(--muted)">(${g.compatibility}%)</span></div>`).join('');
    $('#cmpBox').innerHTML = `<div class="table-wrap" style="margin-top:20px"><table class="cmp"><thead><tr><th>Metric</th><th>Greedy grouping</th><th>BFS / DFS grouping</th></tr></thead><tbody>
      ${row('Groups generated', a.groups, b.groups)}${row('Students assigned', a.studentsAssigned, b.studentsAssigned, true, v => v + ' / ' + a.totalStudents)}
      ${row('Average compatibility', a.avgCompatibility, b.avgCompatibility, true, v => v + '%')}${row('Subject coverage', a.subjectsCovered, b.subjectsCovered, true, v => v + ' of ' + a.totalSubjects)}
      <tr><th>Groups</th><td>${list(d.greedy)}</td><td>${list(d.bfs)}</td></tr></tbody></table></div><p style="margin-top:10px;font-size:12.5px;color:var(--muted)">Green = better value. Comparison is not saved; use Generate to keep a result.</p>`;
  } catch (e) { toast(e.message, 'err'); }
}

/* ---------------- generated groups ---------------- */
async function renderGroups() {
  groupsData = await api('GET', '/api/groups'); const d = groupsData, el = $('#page-groups'), m = d.metrics;
  if (!d.groups.length) { el.innerHTML = `<div class="card"><div class="empty"><div class="big">${IC.groups}</div><h3>No groups generated yet</h3><p style="margin-bottom:16px">Generate study groups from the registered students.</p><button class="btn btn-primary" id="gNow">${IC.bolt} Generate Groups</button> <a class="btn btn-ghost" href="#find">Choose algorithm</a></div></div>`; $('#gNow').onclick = async () => { if (await generate()) renderGroups(); }; return; }
  el.innerHTML = `<div class="card" style="margin-bottom:20px"><div class="card-head" style="margin:0;align-items:center;flex-wrap:wrap">
    <div class="group-meta"><span class="pill-info">${IC.bolt} ${d.algorithm === 'greedy' ? 'Greedy + DSU' : 'BFS components'}</span><span class="pill-info">${IC.groups} ${m.groups} groups</span><span class="pill-info">${IC.user} ${m.studentsAssigned}/${m.totalStudents} students placed</span><span class="pill-info">${IC.star} avg ${m.avgCompatibility}%</span><span class="pill-info">${IC.clock} ${esc(d.generatedAt)}</span></div>
    <div style="display:flex;gap:10px"><button class="btn btn-primary" id="regen">${IC.bolt} Regenerate</button><button class="btn btn-danger" id="clearG">${IC.trash} Clear</button></div></div></div>
  <div class="grid g2" id="gGrid">${d.groups.map(groupCard).join('')}</div>`;
  animateBars(el);
  $('#regen').onclick = async () => { gen.algorithm = d.algorithm || gen.algorithm; gen.threshold = d.threshold; gen.maxSize = d.maxSize; if (await generate()) renderGroups(); };
  $('#clearG').onclick = () => confirmBox('Clear all groups?', 'All generated groups will be removed. Students are not deleted.', async () => { await api('DELETE', '/api/groups'); renderGroups(); });
  $$('[data-rm]', el).forEach(b => b.onclick = () => { const [gid, sid] = b.dataset.rm.split(':'); confirmBox('Remove from group?', 'The student will become unassigned. Groups left with fewer than 2 members are dissolved.', async () => { try { await api('DELETE', `/api/groups/${gid}/members/${sid}`); toast('Student removed from group'); renderGroups(); } catch (e) { toast(e.message, 'err'); } }); });
  $$('[data-view]', el).forEach(b => b.onclick = () => viewProfile(+b.dataset.view));
}
function groupCard(g) {
  const days = g.dayCounts, lv = g.levels, tot = g.size;
  return `<div class="group"><div class="group-head"><div><small>GROUP ${g.number}</small><h3>${esc(g.name)}</h3><small>${g.size} members</small></div>${ring(g.compatibility, 74, 8, '#7dd3fc', 'rgba(255,255,255,.2)')}</div>
  <div class="group-body">
   <div><div class="sec-t">Members</div>${g.members.map(s => `<div class="member">${avatar(s)}<div class="meta"><b style="cursor:pointer" data-view="${s.id}">${esc(s.name)}</b><span>${esc(s.studentId)} · ${esc(s.skills.slice(0, 3).join(', ') || 'no skills listed')}</span></div><span class="badge b-${s.level}">${s.level}</span><button class="icon-btn del" title="Remove from group" data-rm="${g.id}:${s.id}">${IC.x}</button></div>`).join('')}</div>
   <div><div class="sec-t">Why these students?</div><ul class="reasons">${g.reasons.map(r => `<li>${esc(r)}</li>`).join('')}</ul></div>
   <div><div class="sec-t">Common subjects</div>${g.sharedSubjects.length ? `<div class="mini-tags">${g.sharedSubjects.map(x => `<span class="hl">${esc(x.subject)} · ${x.count}/${g.size}</span>`).join('')}</div>` : '—'}</div>
   <div><div class="sec-t">Availability (members per day) · best window ${esc(g.availability.window)}</div><div class="days">${days.map(d => `<div><i style="--h:${d.count / g.size * 100}%"></i>${d.day}</div>`).join('')}</div></div>
   <div><div class="sec-t">Experience levels</div><div class="stack">${['Beginner', 'Intermediate', 'Advanced'].map(k => lv[k] ? `<i style="flex:${lv[k]};background:${LV[k]}" title="${k}: ${lv[k]}"></i>` : '').join('')}</div><div style="font-size:12.5px;color:var(--muted);margin-top:6px">${['Beginner', 'Intermediate', 'Advanced'].map(k => `${lv[k]} ${k}`).join(' · ')}</div></div>
  </div></div>`;
}

/* ---------------- algorithm analysis ---------------- */
const ST = `<table><tr><th>Student</th><th>Subjects</th><th>Days</th></tr><tr><td>A</td><td>DSA, DBMS</td><td>Mon, Wed</td></tr><tr><td>B</td><td>DSA</td><td>Mon</td></tr><tr><td>C</td><td>DSA, Maths</td><td>Wed</td></tr><tr><td>D</td><td>Maths</td><td>Fri</td></tr><tr><td>E</td><td>DBMS</td><td>Mon</td></tr></table>`;
const TOPICS = [
  { t: 'Graph', tag: 'Students as vertices, compatibility as edges', def: 'A graph G = (V, E) is a set of vertices connected by edges. Here every student is a vertex and an edge joins two students who are compatible.', why: 'Relationships between students (who can study with whom) are naturally a graph. Once the graph exists, BFS, DSU and greedy selection can work on it.', how: ['Create one vertex per student.', 'Calculate the compatibility score of every pair (n·(n−1)/2 pairs).', 'If the pair shares a subject, has overlapping time and the score ≥ threshold, add an undirected edge.'], ex: `${ST}<p>Edges (shared subject + shared time): <span class="cx">A–B (DSA, Mon)</span><span class="cx">A–C (DSA, Wed)</span><span class="cx">A–E (DBMS, Mon)</span></p><p>B–C share DSA but no common day, D–C share Maths but no common day → no edge.</p>`, code: `for i in 0..n-1:\n  for j in i+1..n-1:\n    c = compatibility(S[i], S[j])\n    if c.hasCommonSubject and c.hasCommonTime and c.score >= T:\n        addEdge(i, j, c.score)`, cx: 'Building the graph: O(V²) pair comparisons' },
  { t: 'Adjacency List', tag: 'How the graph is stored in C++', def: 'An adjacency list keeps, for every vertex, a list of its neighbours (and the edge weight). In C++: vector<vector<Edge>> adj.', why: 'Our graph is sparse (most students are not compatible with everybody), so a list uses less memory than a V×V matrix and BFS only visits real neighbours.', how: ['adj[u] holds a list of {neighbour, weight}.', 'An undirected edge is stored twice: adj[u] gets v and adj[v] gets u.', 'To visit neighbours of u, loop over adj[u].'], ex: `<p>For the example graph:</p><div class="code">A → B(80), C(70), E(55)\nB → A(80)\nC → A(70)\nD → (none)\nE → A(55)</div>`, code: `struct Edge { int to; double weight; };\nvector<vector<Edge>> adj(n);\n\nvoid addEdge(u, v, w):\n    adj[u].push_back({v, w})\n    adj[v].push_back({u, w})`, cx: 'Space O(V + E) · add edge O(1) · visit all neighbours of u: O(deg(u))' },
  { t: 'Weighted Edges', tag: 'Compatibility score out of 100', def: 'A weighted edge stores a number (here the compatibility score) along with the connection.', why: 'The weight tells us how good a match is, so the greedy algorithm can process the strongest pairs first.', how: ['Common subjects → 40 points.', 'Common timings (shared days × overlapping hours) → 30 points.', 'Shared learning goals → 20 points.', 'Complementary skills and different levels → 10 points.', 'Total = sum of the four parts (0–100).'], ex: `<div class="formula"><div><b>40</b><span>Common subjects</span></div><div><b>30</b><span>Common timings</span></div><div><b>20</b><span>Shared goals</span></div><div><b>10</b><span>Complementary skills</span></div></div><p>Student A (DSA, DBMS, Maths; Mon/Wed/Fri 4–6 PM) and B (DSA, DBMS; Mon/Wed 4–5 PM): subjects 2/2 → 40, days 2/2 × hours 1/1 → 30, then goals and skills add the rest.</p>`, code: `subject = 40 * common / min(|A.subjects|, |B.subjects|)\ntiming  = 30 * (commonDays/minDays) * (overlapHrs/minHrs)\ngoals   = 20 * commonGoals / min(|A.goals|, |B.goals|)\nskills  = 7 * uniqueSkills/allSkills + (3 if levels differ)\nscore   = subject + timing + goals + skills`, cx: 'One pair: O(s²) for small lists (s = list size, ≈ constant)' },
  { t: 'Greedy Algorithm', tag: 'Best pairs first, never undo a choice', def: 'A greedy algorithm always takes the locally best option (here the highest-scoring pair) and never reconsiders it.', why: 'It is simple, fast and produces strong groups: the closest study partners are put together first.', how: ['Collect all edges from the graph.', 'Sort edges by weight, highest first.', 'For each edge, if the two students are in different groups, merge the groups using DSU – provided the merged group is not above the preferred size and all members still share a subject.', 'Groups with at least 2 students are the result.'], ex: `<p>Edges sorted: A–B (80), A–C (70), A–E (55). Max size 3.</p><p><b>1.</b> A–B → group {A,B}. <b>2.</b> A–C → merge → {A,B,C} (all share DSA, size 3 ✔). <b>3.</b> A–E → size would be 4 > 3 ✘ skipped. D stays unassigned.</p><span class="cx">Result: {A, B, C}</span>`, code: `sort(edges, by weight, descending)\nfor each edge (u, v):\n    ru = find(u); rv = find(v)\n    if ru == rv: continue\n    merged = members[ru] + members[rv]\n    if size(merged) > limit: continue\n    if no common subject in merged: continue\n    union(ru, rv)`, cx: 'Sorting O(E log E) + merging O(E · V) → fine for class-size data' },
  { t: 'BFS / DFS', tag: 'Finding connected students', def: 'Breadth-First Search visits a vertex, then all its neighbours, then their neighbours, using a queue. It finds every vertex reachable from a start vertex.', why: 'Students who are linked through compatible pairs form a connected component. BFS finds these clusters without comparing every pair again.', how: ['Pick an unvisited student, push into a queue.', 'Pop a student, record it, push all unvisited neighbours.', 'When the queue is empty, one connected component is complete.', 'Large components are cut into pieces of the preferred size (in BFS order); only members sharing the piece\'s main subject are kept.'], ex: `${ST}<p>Graph edges: A–B, A–C, A–E. BFS from A visits <b>A, B, C, E</b> (queue order). D has no edge → separate component of size 1 (ignored).</p><span class="cx">Component: {A, B, C, E}</span><p>With max size 3 → piece {A,B,C}, then {E} alone (dropped).</p>`, code: `visited[] = false\nfor each student s:\n    if not visited[s]:\n        queue.push(s); visited[s] = true\n        while queue not empty:\n            u = queue.pop(); component.add(u)\n            for each (v, w) in adj[u]:\n                if not visited[v]: visited[v] = true; queue.push(v)`, cx: 'O(V + E) with an adjacency list' },
  { t: 'Disjoint Set Union', tag: 'Keeping track of groups', def: 'DSU (Union-Find) keeps elements in disjoint sets. find(x) returns the set leader and union(a, b) merges two sets. Path compression makes later finds almost O(1).', why: 'The greedy algorithm needs to know instantly "are these two students already in the same group?" and to merge groups. DSU guarantees every student belongs to exactly one set, so nobody is placed in two groups.', how: ['parent[i] = i at the start (everyone is alone).', 'find(x) follows parents to the root and shortens the path (path compression).', 'union(a, b) attaches the smaller set under the larger set.'], ex: `<p>Start: {A} {B} {C} {D} {E}</p><p>union(A,B) → {A,B} {C} {D} {E}<br>union(A,C) → {A,B,C} {D} {E}<br>find(B) == find(C) → true → already one group, skip.</p><span class="cx">Final sets: {A,B,C} {D} {E}</span>`, code: `int find(x):\n    if parent[x] != x:\n        parent[x] = find(parent[x])   // path compression\n    return parent[x]\n\nbool unite(a, b):\n    ra = find(a); rb = find(b)\n    if ra == rb: return false\n    parent[rb] = ra; size[ra] += size[rb]\n    return true`, cx: 'Almost O(1) per operation (inverse Ackermann) · Space O(V)' }
];
let anTab = 0;
function renderAnalysis() {
  const el = $('#page-analysis');
  const draw = () => { const t = TOPICS[anTab];
    el.innerHTML = `<div class="an-layout"><div class="an-tabs">${TOPICS.map((x, i) => `<button class="${i === anTab ? 'on' : ''}" data-i="${i}"><span>${i + 1}</span>${x.t}</button>`).join('')}</div>
    <div class="card an"><h2>${t.t}</h2><div class="tagline">${t.tag}</div><h4>Definition</h4><p>${t.def}</p><h4>Why it is used in this project</h4><p>${t.why}</p><h4>How it works</h4><ol>${t.how.map(h => `<li>${h}</li>`).join('')}</ol>
    <h4>Worked example (5 students)</h4><div class="example">${t.ex}</div><h4>Pseudocode</h4><div class="code">${esc(t.code)}</div><h4>Time complexity</h4><span class="cx">${t.cx}</span></div></div>`;
    $$('.an-tabs button', el).forEach(b => b.onclick = () => { anTab = +b.dataset.i; draw(); window.scrollTo(0, 0); }); };
  draw();
}

/* ---------------- about ---------------- */
function renderAbout() {
  $('#page-about').innerHTML = `<div class="banner" style="margin-bottom:20px"><div><h2>Smart Study Group Formation System</h2><p>A second-year Computer Engineering DSA mini-project that recommends study partners and forms study groups using graphs, greedy selection, BFS and DSU – all implemented in C++.</p></div></div>
  <div class="grid g2"><div class="card"><h3>Architecture</h3><div class="sub">Everything runs locally</div><div class="about-list"><div><b>Frontend</b>HTML5 · CSS3 · Vanilla JavaScript</div><div><b>Backend</b>C++17 (own HTTP server)</div><div><b>API</b>REST + JSON on port 8080</div><div><b>Storage</b>JSON files in /data</div></div></div>
  <div class="card"><h3>Compatibility formula</h3><div class="sub">Out of 100</div><div class="formula" style="grid-template-columns:repeat(2,1fr)"><div><b>40%</b><span>Common subjects</span></div><div><b>30%</b><span>Common timings</span></div><div><b>20%</b><span>Shared goals</span></div><div><b>10%</b><span>Complementary skills</span></div></div></div></div>
  <div class="card mt"><h3>REST API</h3><div class="sub">Implemented in backend/main.cpp</div><div class="table-wrap"><table class="endpoints"><thead><tr><th>Method</th><th>Endpoint</th><th>Purpose</th></tr></thead><tbody>
  ${[['GET/POST', '/api/students', 'List / register students'], ['PUT/DELETE', '/api/students/:id', 'Edit / delete a student'], ['GET', '/api/stats', 'Dashboard statistics'], ['GET', '/api/compatibility/:id', 'Scores of one student vs all'], ['GET', '/api/graph?threshold=', 'Adjacency list, edges, BFS/DSU component counts'], ['POST', '/api/groups/generate', 'Run Greedy or BFS grouping and save'], ['POST', '/api/groups/compare', 'Run both algorithms (not saved)'], ['GET/DELETE', '/api/groups', 'View / clear groups'], ['DELETE', '/api/groups/:gid/members/:sid', 'Remove a student from a group']].map(r => `<tr><td><b>${r[0]}</b></td><td><code>${r[1]}</code></td><td>${r[2]}</td></tr>`).join('')}</tbody></table></div></div>`;
}
route();

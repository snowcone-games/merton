// Globals

const e = React.createElement;

const OPACITY = '0.97';

const SYSTEMS = {
	atari2600: {
		name: 'Atari 2600',
		cores: ['stella', 'stella_libretro']
	},
	nes: {
		name: 'NES',
		cores: ['mesen2', 'merton-nes', 'mesen_libretro']
	},
	ms: {
		name: 'Master System',
		cores: ['mesen2', 'genesis_plus_gx_libretro']
	},
	tg16: {
		name: 'TurboGrafx-16',
		cores: ['mesen2', 'mednafen_pce_libretro']
	},
	genesis: {
		name: 'Genesis',
		cores: ['genesis-plus-gx', 'genesis_plus_gx_libretro']
	},
	gameboy: {
		name: 'Game Boy',
		cores: ['mesen2', 'sameboy_libretro']
	},
	snes: {
		name: 'SNES',
		cores: ['mesen2', 'snes9x_libretro', 'bsnes_libretro', 'mesen-s_libretro']
	},
	ss: {
		name: 'Saturn',
		cores: ['mednafen_saturn_libretro'],
	},
	ps: {
		name: 'PlayStation',
		cores: ['duckstation', 'swanstation_libretro']
	},
	n64: {
		name: 'N64',
		cores: ['mupen64plus', 'mupen64plus_next_libretro']
	},
	gba: {
		name: 'Game Boy Advance',
		cores: ['mgba', 'mesen2', 'mgba_libretro']
	},
};

const MENU_ITEMS = [
	{name: 'System', items: [
		{name: 'load-rom', type: 'files', etype: 'label', label: 'Load ROM'},
		{name: 'unload-rom', type: 'action', etype: 'label', label: 'Unload ROM', needsRunning: true},
		{name: 'reload', type: 'action', etype: 'label', label: 'Reload ROM', needsRunning: true},
		{name: 'reset', type: 'nstate', etype: 'label', label: 'Reset', needsRunning: true},
		{etype: 'separator'},
		{name: 'save-state', type: 'nstate', etype: 'dropdown', label: 'Save State', needsRunning: true, opts: [
			{label: 'Select Slot', value: '---'},
			{label: '1', value: 1},
			{label: '2', value: 2},
			{label: '3', value: 3},
			{label: '4', value: 4},
			{label: '5', value: 5},
			{label: '6', value: 6},
			{label: '7', value: 7},
			{label: '8', value: 8},
		]},
		{name: 'load-state', type: 'nstate', etype: 'dropdown', label: 'Load State', needsRunning: true, opts: [
			{label: 'Select Slot', value: '---'},
			{label: '1', value: 1},
			{label: '2', value: 2},
			{label: '3', value: 3},
			{label: '4', value: 4},
			{label: '5', value: 5},
			{label: '6', value: 6},
			{label: '7', value: 7},
			{label: '8', value: 8},
		]},
		{etype: 'separator'},
		{name: 'menu_pause', type: 'cfg', etype: 'checkbox', label: 'Menu Pause'},
		{name: 'bg_pause', type: 'cfg', etype: 'checkbox', label: 'Background Pause'},
		{etype: 'separator'},
		{name: 'insert-disc', type: 'discs', etype: 'label', label: 'Insert Disc', needsRunning: true, needsDisks: true},
		{name: 'add-bios', type: 'bios', etype: 'label', label: 'Add BIOS'},
		{etype: 'separator'},
		{name: 'quit', type: 'action', etype: 'label', label: 'Quit'},
	]},
	{name: 'Video', items: [
		{name: 'reset-window', type: 'action', etype: 'label', label: 'Reset Window', needsWindowAdjustments: true},
		{etype: 'separator', needsWindowAdjustments: true},
		{name: 'fullscreen', type: 'cfg', etype: 'checkbox', label: 'Fullscreen', needsWindowAdjustments: true},
		{name: 'square_pixels', type: 'cfg', etype: 'checkbox', label: 'Square Pixels'},
		{name: 'int_scaling', type: 'cfg', etype: 'checkbox', label: 'Integer Scaling'},
		{etype: 'separator'},
		{name: 'vsync', type: 'cfg', etype: 'dropdown', label: 'VSync', opts: [
			{label: 'Off', value: 0},
			{label: 'Auto', value: -1},
			{label: '1.00 (60 Hz)', value: 100},
			{label: '1.25 (75 Hz)', value: 125},
			{label: '2.00 (120 Hz)', value: 200},
			{label: '2.40 (144 Hz)', value: 240},
			{label: '4.00 (240 Hz)', value: 400},
		]},
		{name: 'filter', type: 'cfg', etype: 'dropdown', label: 'Filter', opts: [
			{label: 'Nearest', value: 0},
			{label: 'Linear', value: 1},
		]},
		{name: 'sharpen', type: 'cfg', etype: 'dropdown', label: 'Sharpen', needsLinear: true, opts: [
			{label: 'None', value: 0},
			{label: 'Low', value: 25},
			{label: 'High', value: 75},
		]},
		{name: 'scanlines', type: 'cfg', etype: 'dropdown', label: 'Scanlines', opts: [
			{label: 'Off', value: 0},
			{label: '60%', value: 60},
			{label: '70%', value: 70},
			{label: '80%', value: 80},
			{label: '90%', value: 90},
			{label: '95%', value: 95},
		]},
	]},
	{name: 'Audio', items: [
		{name: 'mute', type: 'cfg', etype: 'checkbox', label: 'Mute'},
		{etype: 'separator'},
		{name: 'audio_buffer', type: 'cfg', etype: 'dropdown', label: 'Audio Buffer', opts: [
			{label: '25 ms', value: 25},
			{label: '50 ms', value: 50},
			{label: '75 ms', value: 75},
			{label: '100 ms', value: 100},
			{label: '125 ms', value: 125},
			{label: '150 ms', value: 150},
		]},
	]},
	{name: 'Advanced', items: [
		{name: 'pause', type: 'nstate', etype: 'checkbox', label: 'Pause Emulation', needsRunning: true},
		{name: 'console', type: 'cfg', etype: 'checkbox', label: 'Console Window'},
		{etype: 'separator'},
		{name: 'gfx', type: 'cfg', etype: 'dropdown', label: 'Graphics', opts: [
			{label: 'OpenGL', value: 1},
			{label: 'Vulkan', value: 2},
			{label: 'D3D11', value: 3},
			{label: 'D3D12', value: 4},
			{label: 'Metal', value: 5},
		]},
		{name: 'playback_rate', type: 'cfg', etype: 'dropdown', label: 'Playback Rate', opts: [
			{label: '44.1 kHz', value: 44100},
			{label: '48 kHz', value: 48000},
			{label: '96 kHz', value: 96000},
		]},
		{etype: 'separator'},
		...systemsToMenu(),
	]},
];


// localStorage

function getLocal(key, def) {
	return localStorage[key] != undefined ? localStorage[key] : def;
}

function setLocal(key, val) {
	localStorage[key] = val;
}


// Events

function handleEvent(evt) {
	switch (evt.type) {
		// Handled internally
		case 'bios':
		case 'discs':
		case 'files':
			handleEvent({type: 'action', name: evt.type, basedir: getLocal('fdir', '')});
			break;

		// Sent to native layer
		case 'cfg':
		case 'action':
		case 'nstate':
		case 'core_opts':
			if (evt.type == 'core_opts')
				evt.value = evt.value.toString(); // Boolean values need to be strings

			if (window.MTY_NativeSendText)
				window.MTY_NativeSendText(JSON.stringify(evt));
			break;
	}
}


// Button

function ActionButton(props) {
	let style = {
		cursor: 'pointer',
		margin: '0.3rem auto 0 auto',
		padding: '.4rem',
		color: `rgba(190, 190, 190, 1.0)`,
	};

	if (props.style)
		style = {...style, ...props.style};

	if (props.disabled) {
		style.color = `rgba(120, 120, 120, 1.0)`;
		style.cursor = '';
		props.onClick = () => {};
	}

	return e('div', {style: style, onClick: props.onClick, disabled: props.disabled,
		'nav-item': props.navGroup, tabindex: props.disabled ? false : -1}, props.label);
}


// Select

function Select(props) {
	const mitem = props.mitem;

	const style = {
		margin: '1.1rem 0 1.2rem 0',
		color: `rgba(190, 190, 190, 1.0)`,
	};

	let lstyle = {
		textTransform: 'uppercase',
		fontSize: '.7rem',
		fontWeight: 'bold',
	};

	let sstyle = {
		background: 'rgba(70, 70, 70, 1)',
		margin: '.4rem 0 0 0',
		padding: '.4rem',
		cursor: 'pointer',
		color: '#CCC',
	};

	let onClick = (evt) => {
		let rect = evt.target.getBoundingClientRect();
		props.setAppState({select: {mitem, selected: props.selected, offset: rect.y}});
	}

	let disabled = props.disabled || mitem.opts.length <= 1;

	if (disabled) {
		sstyle.color = lstyle.color = 'rgba(120, 120, 120, 1.0)';
		sstyle.background = '#444';
		sstyle.cursor = '';
		onClick = () => {};
	}

	let selected = mitem.opts[0].label;

	for (let x = 0; x < mitem.opts.length; x++)
		if (mitem.opts[x].value == props.selected)
			selected = mitem.opts[x].label;

	return e('div', {style: style}, [
		e('div', {style: lstyle}, mitem.label),
		e('div', {style: sstyle, disabled, onClick: onClick, 'nav-item': 1,
			tabindex: disabled ? false : -1}, selected),
	]);
}

function clearModals(setAppState, group) {
	setAppState({select: null, files: null});
	NAV_CaptureLeft(null);
	NAV_ResetGroup(2);

	if (group != -1)
		NAV_SwitchGroup(group)
}

class SelectMenu extends React.Component {
	constructor(props) {
		super(props);
	}

	componentDidMount() {
		const mitem = this.props.appState.select.mitem;
		const selected = this.props.appState.select.selected;

		for (let x = 0, dummy = 0; x < mitem.opts.length; x++) {
			if (mitem.opts[x].value == '---')
				dummy++;

			if (mitem.opts[x].value == selected) {
				NAV_Focus(2, x - dummy);
				return;
			}
		}

		NAV_Focus(2, 0);
	}

	render() {
		const mitem = this.props.appState.select.mitem;
		const selected = this.props.appState.select.selected;

		let offset = this.props.appState.select.offset * 0.95;
		let len = mitem.opts.length;
		let h = len * 0.035;
		let bottom = offset / window.innerHeight + h;

		if (bottom + h > 0.95)
			offset -= (bottom + h - 0.95) * window.innerHeight;

		if (offset < 0)
			offset = 0;

		const border = `.025rem solid rgba(70, 70, 70, ${OPACITY})`;

		const style = {
			verticalAlign: 'top',
			position: 'relative',
			maxHeight: '100%',
			top: `${offset}px`,
			minWidth: '8rem',
			padding: '.7rem 1rem',
			background: `rgba(55, 55, 55, ${OPACITY})`,
			borderTop: border,
			borderRight: border,
			borderBottom: border,
			boxSizing: 'border-box',
			display: 'inline-block',
			overflowY: 'auto',
		};

		let items = [];

		for (let x = 0; x < mitem.opts.length; x++) {
			if (mitem.opts[x].value == '---')
				continue;

			let style = {};

			if (mitem.opts[x].value == selected) {
				style = {
					color: 'rgba(100, 100, 220, 1)',
					fontWeight: 'bold',
				}
			}

			items.push(e(ActionButton, {
				disabled: false,
				onClick: () => {
					handleEvent({name: mitem.name, type: mitem.type, value: mitem.opts[x].value});
					this.props.setValue(mitem.type == 'core_opts' ? 'cfg' : mitem.type, mitem.name, mitem.opts[x].value);
					clearModals(this.props.setAppState, 1);
				},
				label: mitem.opts[x].label,
				navGroup: 2,
				style: style,
			}));
		}

		return e('div', {style: style}, items);
	}
}


// Checkbox

function Checkbox(props) {
	const mitem = props.mitem;

	let style = {
		cursor: 'pointer',
		margin: '.3rem 0 0 0',
		padding: '.4rem',
		color: `rgba(190, 190, 190, 1.0)`,
		display: 'block',
	};

	let cbstyle = {
		margin: '0 .5rem 0 0',
		padding: '0',
		width: '.8rem',
		height: '.8rem',
		position: 'relative',
		pointerEvents: 'none',
		top: '.08rem',
	};

	let onChange = (evt) => {
		handleEvent({name: mitem.name, type: mitem.type, value: evt.target.checked});
		props.setValue(mitem.type == 'core_opts' ? 'cfg' : mitem.type, mitem.name, evt.target.checked);
	};

	let onClick = (evt) => {
		evt.preventDefault();

		let input = evt.target.control;

		if (!input.disabled) {
			input.checked = !input.checked;
			onChange({target: input});
		}
	};

	if (props.disabled) {
		style.color = `rgba(120, 120, 120, 1.0)`;
		style.cursor = '';
		onChange = () => {};
	}

	let labelProps = {style: style, onClick: onClick, disabled: props.disabled,
		'nav-item': 1, tabindex: props.disabled ? false : -1};

	let checkboxProps = {style: cbstyle, type: 'checkbox', onChange: onChange,
			checked: props.checked, disabled: props.disabled};

	return e('label', labelProps, e('input', checkboxProps), mitem.label);
}


// Separator

function Separator() {
	const style = {
		height: '.025rem',
		width: '100%',
		background: `rgba(100, 100, 100, ${OPACITY})`,
		margin: '.6rem 0',
	};

	return e('div', {style: style});
}


// Menu

function MenuButton(props) {
	let style = {
		cursor: 'pointer',
		margin: '.3rem auto 0 auto',
		padding: '.4rem',
	};

	if (props.selected)
		style.background = `rgba(70, 70, 70, ${OPACITY})`;

	let onClick = () => {
		if (!props.selected) {
			NAV_ResetGroup(1);
			props.setAppState({menuIndex: props.index});
		}
	}

	if (props.disabled) {
		style.cursor = '';
		style.color = 'rgba(120, 120, 120, 1.0)';
		onClick = () => {};
	}

	return e('div', {style: style, onClick: onClick, disabled: props.disabled,
		'nav-item': 0, 'nav-auto': 1, focus: 1, tabindex: props.disabled ? false : -1}, props.name);
}

function MenuLeft(props) {
	const style = {
		width: '10rem',
		height: '100%',
		padding: '.7rem 1rem',
		background: `rgba(40, 40, 40, ${OPACITY})`,
		borderRight: `.025rem solid rgba(70, 70, 70, ${OPACITY})`,
		boxSizing: 'border-box',
		float: 'left',
	};

	let items = [];

	for (let x = 0; x < props.appState.menuItems.length; x++) {
		const item = props.appState.menuItems[x];
		const disabled = item.needsRunning && !props.appState.nstate.running;
		items.push(e(MenuButton, {index: x, name: item.name, selected: props.appState.menuIndex == x,
			setAppState: props.setAppState, disabled: disabled}));
	}

	return e('div', {style: style}, items);
}

function stringBoolConversion(val) {
	if (typeof val == 'string')
		return val.toLowerCase() === 'true';

	return val;
}

function MenuRight(props) {
	const style = {
		width: '15rem',
		height: '100%',
		padding: '.7rem 1.2rem',
		background: `rgba(55, 55, 55, ${OPACITY})`,
		borderRight: `.025rem solid rgba(70, 70, 70, ${OPACITY})`,
		boxSizing: 'border-box',
		display: 'inline-block',
		overflowY: 'auto',
	};

	const menuItems = props.appState.menuItems[props.appState.menuIndex].items;

	let items = [];

	for (let x = 0; x < menuItems.length; x++) {
		const mitem = menuItems[x];

		if (!props.appState.nstate.allow_window_adjustments && mitem.needsWindowAdjustments)
			continue;

		const disabled = (!props.appState.nstate.running && mitem.needsRunning) ||
			(!props.appState.nstate.has_discs && mitem.needsDisks) ||
			(props.appState.cfg.filter != 1 && mitem.needsLinear);

		switch (mitem.etype) {
			case 'label':
				items.push(e(ActionButton, {
					disabled: disabled,
					onClick: () => handleEvent({name: mitem.name, type: mitem.type},
						props.appState, props.setAppState),
					label: mitem.label,
					navGroup: 1,
				}));
				break;
			case 'checkbox': {
				let val = props.appState[mitem.type == 'core_opts' ? 'cfg' : mitem.type][mitem.name];
				val = stringBoolConversion(val);

				items.push(e(Checkbox, {mitem: mitem, setValue: props.setValue, checked: val, disabled: disabled}));
				break;
			}
			case 'dropdown': {
				const val = props.appState[mitem.type == 'core_opts' ? 'cfg' : mitem.type][mitem.name];
				items.push(e(Select, {mitem: mitem, setAppState: props.setAppState, selected: val, disabled: disabled}));
				break;
			}
			case 'separator':
				items.push(e(Separator));
				break;
		}
	}

	return e('div', {style: style}, items);
}


// Modals

function Modal(props) {
	const ostyle = {
		position: 'absolute',
		top: '0',
		width: '100%',
		height: '100%',
		background: 'rgba(0, 0, 0, .5)',
	};

	const cstyle = {
		position: 'absolute',
		top: '10%',
		left: '15%',
		width: '70%',
		height: '80%',
		padding: '1.5rem',
		background: '#444',
		boxSizing: 'border-box',
		border: '0.025rem solid #666',
		boxShadow: '-.12rem .12rem .5rem .25rem rgba(0, 0, 0, .2)',
	};

	const tstyle = {
		textTransform: 'uppercase',
		fontWeight: 'bold',
		color: '#CCC',
	};

	let cancel = () => {
		clearModals(props.setAppState, 1);
	};

	let cclick = (evt) => {
		evt.stopPropagation();
	};

	return e('div', {style: ostyle, onClick: cancel}, // Dimmed overlay
		e('div', {style: cstyle, onClick: cclick}, [ // Modal container
			e('div', {style: tstyle}, props.title),
			props.children,
		]),
	);
}

class LoadROMModal extends React.Component {
	constructor(props) {
		super(props);
	}

	componentDidMount() {
		NAV_Focus(2, getLocal('findex', 0));
	}

	render() {
		let files = this.props.appState.files.list;
		let dir = this.props.appState.files.path;
		let type = this.props.appState.files.type;
		let actionType = type == 'files' ? 'load-rom' : type == 'discs' ? 'insert-disc' :
			'add-bios';
		let title = type == 'files' ? 'Load ROM' : type == 'discs' ? "Insert Disc" : 'Add BIOS';

		setLocal('fdir', dir);

		NAV_CaptureLeft(() => {
			handleEvent({type: 'action', name: type, basedir: dir, dir: '..'});
		});

		const rstyle = {
			padding: '0.3rem',
			borderBottom: '.025rem solid #666',
			fontWeight: 'bold',
			cursor: 'pointer',
		};

		const rfstyle = {
			...rstyle,
			color: '#CCC',
			fontWeight: 'normal',
		};

		const cstyle = {
			padding: '.5rem',
			overflowY: 'auto',
			height: '80%',
		};

		const dstyle = {
			margin: '1.2rem 0 .5rem .5rem',
		};

		let items = [];

		for (let x = 0; x < files.length; x++) {
			let file = files[x];

			if (file.name == '.')
				continue;

			let click = () => {
				if (file.dir) {
					handleEvent({type: 'action', name: type, basedir: dir, dir: file.name});
					setLocal('findex', 0);
					NAV_Focus(2, 0);

				} else {
					handleEvent({type: 'action', name: actionType, basedir: dir, fname: file.name});
					setLocal('findex', x - 1); // For '.'
					clearModals(this.props.setAppState, 1);
				}
			};

			items.push(e('div', {style: file.dir ? rstyle: rfstyle, onClick: click, 'nav-item': 2, tabindex: -1}, file.name));
		}

		return e(Modal, {title: title, setAppState: this.props.setAppState},
			[
				e('div', {style: dstyle}, dir),
				e('div', {style: cstyle}, items),
			],
		);
	}
}


// Main

function Body(props) {
	const style = {
		width: '100%',
		height: '100%',
		color: 'rgba(230, 230, 230, 1.0)',
		letterSpacing: '.03rem',
		fontSize: '.9rem',
		overflow: 'hidden',
		fontFamily: 'sans-serif',
	};

	const setValue = (type, key, val) => {
		const obj = props.appState[type];
		obj[key] = val;

		props.setAppState({type: obj});
	};

	return [
		e('div', {style: style}, [
			e(MenuLeft, {appState: props.appState, setAppState: props.setAppState}),
			e(MenuRight, {appState: props.appState, setAppState: props.setAppState, setValue: setValue}),
			props.appState.select ? e(SelectMenu, {appState: props.appState, setAppState:
				props.setAppState, setValue: setValue}) : null,
			props.appState.files ? e(LoadROMModal, {appState: props.appState, setAppState: props.setAppState}) : null,
		]),
	];
}

function systemsToMenu() {
	let items = [];

	const keys = Object.keys(SYSTEMS);

	for (let x = 0; x < keys.length; x++) {
		const system = keys[x];
		const info = SYSTEMS[system];

		let mitem = {name: `core.${system}`, type: 'cfg', etype: 'dropdown', label: info.name, opts: []};

		for (let y = 0; y < info.cores.length; y++) {
			const name = info.cores[y];
			mitem.opts.push({label: name, value: name});
		}

		items.push(mitem);
	}

	return items;
}

function coreOptsToMenu(core_opts) {
	let mrow = {name: 'Core Menu', needsRunning: true, items: []};

	mrow.items.push({name: 'core-reset', type: 'action', etype: 'label', label: 'Reset Core Settings'});
	mrow.items.push({etype: 'separator'});

	for (let x = 0; x < core_opts.length; x++) {
		const s = core_opts[x];
		console.log(s);

		let mitem = {name: s.key, type: 'core_opts', etype: s.type, label: s.desc};

		if (s.type == 'dropdown') {
			mitem.opts = [];

			for (let y = 0; y < s.list.length; y++) {
				const opt = s.list[y];
				mitem.opts.push({label: opt, value: opt});
			}
		}

		mrow.items.push(mitem);
	}

	return mrow;
}

class Main extends React.Component {
	constructor(props) {
		super(props);

		const setAppState = (state) =>
			this.setState(state);

		this.state = {
			select: null,
			files: null,
			menuIndex: 0,
			menuItems: MENU_ITEMS,
			cfg: {},
			core_opts: {},
			nstate: {},
		};

		NAV_SetLoseFocus((element) => {
			if (this.state.select) {
				if (!element) {
					clearModals(setAppState, 1);

				} else if (element.getAttribute('nav-item') != 2) {
					clearModals(setAppState, -1);
				}
			}
		});

		NAV_SetCancel(() => {
			if (this.state.select || this.state.files) {
				clearModals(setAppState, 1);

			} else {
				handleEvent({type: 'action', name: 'hide-menu'});
			}
		});

		NAV_SetScroll((dir) => {
			if (this.state.files)
				NAV_FocusRelative(5 * dir);
		});

		window.MTY_NativeListener = msg => {
			const json = JSON.parse(msg);

			switch (json.type) {
				case 'state': {
					this.setState(json);

					let menuItems = [...MENU_ITEMS];

					menuItems.push(coreOptsToMenu(json.core_opts));

					this.setState({menuItems: menuItems});
					break;
				}
				case 'controller':
					NAV_Controller(json);
					break;
				case 'bios':
				case 'discs':
				case 'files':
					this.setState({files: json});
					break;
			}
		};
	}

	render() {
		const setAppState = (state) =>
			this.setState(state);

		return e(Body, {appState: this.state, setAppState: setAppState});
	}
}

(function main() {
	document.oncontextmenu = (e) => {
		e.preventDefault();
		return false;
	};

	window.addEventListener('dragover', (e) => {
		e.preventDefault();
	});

	window.addEventListener('drop', (e) => {
		e.preventDefault();
	});

	ReactDOM.render(e(Main), document.body);

	NAV_SetFocusClass('focus');
	NAV_Focus(0, 0);
})();



var words = ['apple', 'beer', 'cake', 'potato', 'orange', 'Got', 'ability', 'shop', 'recall', 'fruit', 'easy', 'dirty', 'giant', 'shaking', 'ground', 'weather', 'lesson', 'almost', 'square', 'forward', 'bend', 'cold', 'broken', 'distant', 'adjective'];

function randomWords() {
  let t = '';
  for (i = 0; i < Math.random() * 100; i++) {
    t += ' ' + words[Math.floor(Math.random() * words.length)];
  }
  return t;
}

const computedStyle = getComputedStyle(document.querySelector(':root'));

const fadeInDuration = parseFloat(
  computedStyle.getPropertyValue('--fade-in-duration')
);
const pushUpDuration = parseFloat(
  computedStyle.getPropertyValue('--push-up-duration')
);
const fadeOutDuration = parseFloat(
  computedStyle.getPropertyValue('--fade-out-duration')
);
const containerInactiveRowGap = parseFloat(
  computedStyle.getPropertyValue('--container-inactive-row-gap')
);

const asyncSleep = (duration) => {
  return new Promise((resolved) => {
    setTimeout(() => {
      resolved();
    }, duration);
  });
};

const transitioned = (element) => {
  return new Promise((resolved) => {
    element.addEventListener('transitionend', (e, ...args) => {
      element.removeEventListener('transitionend', args.callee);
      resolved();
    });
  });
};

// Globals variables for DOM elements
const inactive = document.querySelector('#container-inactive');
const container = document.querySelector('#container');
const offscreenRegion = document.querySelector('#offscreen-region');

async function pushMessage(parsedMessage, timeout) {
  if(parsedMessage.kind !== 1) { 
    console.log(parsedMessage);
    return; 
  }
  const msgParts = parsedMessage.message.parts;
  const displayName = parsedMessage.message.display_name;

  const name = document.createElement("div")
  name.classList.add('display-name');
  name.append(document.createTextNode(displayName));

  let msg = document.createElement("div");
  msg.classList.add('message-content');

  for (const msgPart of msgParts) {
    switch (msgPart.type) {
      case 'text': {
        msg.append(document.createTextNode(msgPart.value));
        break;
      }
      case 'emote': {
        let elem = document.createElement("img");
        elem.setAttribute('src', msgPart.value);
        msg.append(elem);
        break;
      }
    }
  }

  // Create message HTML element
  const message = document.createElement('li');
  message.append(name);
  message.append(msg);
  message.classList.add('message');

  // Add message element into offscreen-region
  offscreenRegion.append(message);

  // Get message's computed height
  const messageHeight =
    message.getBoundingClientRect().height + containerInactiveRowGap;

  // Reset container-inactive's transition duration back to bottomSpanerDuration
  inactive.style['transition-duration'] = pushUpDuration + 's';

  // Pushing up container-inactive with transition based on message's height
  inactive.style['transform'] = `translate(0%, ${-messageHeight}px)`;

  // Wait for container-inactive transform transition to finish
  await transitioned(inactive);

  // Slide and fading in message into container
  message.classList.add('fadeIn');
  container.appendChild(message);

  // Wait for message slide and fading in animation to finish
  await asyncSleep(fadeInDuration * 1000);

  // Don't need fade in animation anymore, remove it
  message.classList.remove('fadeIn');

  // Set container-inactive's transition duration to 0 in order to 
  // push it down without any animation
  inactive.style['transition-duration'] = '0.0s';

  // Push down container-inactive, prepare to move message into it
  inactive.style.removeProperty('transform');

  // Move message into container-inactive
  // Everything visually should be the same before and after this move
  inactive.prepend(message);

  // Wait for specified amount of time for message to be read
  await asyncSleep(timeout * 1000);

  // Fading out message
  message.classList.add('fadeOut');

  // Wait for message fade out animation to finish
  await asyncSleep(fadeOutDuration * 1000);

  // Remove message from the DOM
  message.remove()
}

// minimum delay (+50ms) between messages necessary to not break animations
const minimumDelay = fadeInDuration + pushUpDuration + 0.05;
const pushingDelay = 0.5 + minimumDelay;
const messageTimeout = 15.0;
container.dataset.messageCount = 0;
container.dataset.messageMax = 20;

const ws = new WebSocket('ws://localhost:8040');

ws.onopen = () => {
  console.log('WebSocket connection opened');
}
ws.onmessage = (msg) => {
  console.log('Message: ', msg.data);
  message = JSON.parse(msg.data);
  setTimeout(() => {
    container.dataset.messageCount++;
    pushMessage(message, messageTimeout);
  }, pushingDelay * 1000);
}

ws.onclose = (msg) => {
  console.log('Closed');
}
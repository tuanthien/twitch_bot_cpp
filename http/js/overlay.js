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


async function displayGenericMessage(data, timeout) {
  const msgParts = data.parts;
  const displayName = data.display_name;
  const id = data.id;
  const modified = data.modified;

  const extra = document.createElement("div")
  extra.classList.add('extra');

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
  if(modified) {
    const handlersEle = document.createElement('div');
    handlersEle.classList.add('handlers')
    extra.append(handlersEle);
  }
  // Create message HTML element
  const message = document.createElement('li');
  message.append(extra);
  message.append(name);
  message.append(msg);
  message.classList.add('message');
  
  message.id = "message-" + id;

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

async function handleCppFormater(data) {
  const refMessage = document.getElementById("message-" + data.ref_id);
  refMessage.dataset.cppState = data.state;
  switch (data.state) {
    case 0: {
      const contentEle = refMessage.querySelector('.message-content');
      const formatedCodeEle = document.createElement('pre');
      formatedCodeEle.innerText = data.formatted_code;
      contentEle.innerHTML = "";
      contentEle.append(formatedCodeEle);
      if(refMessage.parentElement.id == 'container-inactive')
        window.scrollTo(0, document.body.scrollHeight);
      break;
    }
    case 1: {
      const cppMsgEle = refMessage.querySelector('.extra > .handlers > .cpp > .message')
      cppMsgEle.innerText = "Error";
      break;
    }
    case 2: {
      refMessage.classList.add('modified');

      const handlersEle = refMessage.querySelector('.extra > .handlers');
   
      const handlerEle = document.createElement('div');
      handlerEle.classList.add('cpp')

      const handlerNameEle = document.createElement('span');
      handlerNameEle.classList.add('name');
      handlerNameEle.innerText = 'cpp';

      const handlerStateEle = document.createElement('span');
      handlerStateEle.classList.add('state');
      handlerStateEle.innerText = 'Formating';

      handlerEle.append(handlerNameEle, handlerStateEle);
      handlersEle.append(handlerEle);
      const handlers = (refMessage.dataset.handlers || "").split(' ');
      handlers.push('cpp');
      refMessage.dataset.handlers = handlers.join(' ');
      break;
    }
    case 3: {
      const cppMsgEle = refMessage.querySelector('.extra > .handlers > .cpp > .message')
      cppMsgEle.innerText = "Timeout";
    }
  }
}

async function pushMessage(parsedMessage, timeout) {
  switch (parsedMessage.kind) {
    case 0: {
      displayGenericMessage(parsedMessage.data, timeout);
      break;
    }
    case 1: {
      handleCppFormater(parsedMessage.data);
      break;
    }
  }

  if (parsedMessage.kind !== 0) {
    return;
  }

}

// minimum delay (+50ms) between messages necessary to not break animations
const minimumDelay = fadeInDuration + pushUpDuration + 0.05;
const pushingDelay = 0.5 + minimumDelay;
const messageTimeout = 100.0;
container.dataset.messageCount = 0;
container.dataset.messageMax = 20;

const ws = new WebSocket('ws://localhost:8040');

ws.onopen = () => {
  console.log('WebSocket connection opened');
}
ws.onmessage = (msg) => {
  console.log('Message: ', msg.data);
  const message = JSON.parse(msg.data);
  setTimeout(() => {
    container.dataset.messageCount++;
    
    pushMessage(message, messageTimeout);
  }, pushingDelay * 1000);

}

ws.onclose = (msg) => {
  console.log('Closed');
}
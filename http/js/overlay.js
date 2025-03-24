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


async function displayGenericMessage(data) {
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

  const msgSpan = document.createElement("span");
  msg.append(msgSpan);

  for (const msgPart of msgParts) {
    switch (msgPart.type) {
      case 'text': {
        msgSpan.append(document.createTextNode(msgPart.value));
        break;
      }
      case 'emote': {
        let elem = document.createElement("img");
        elem.setAttribute('src', msgPart.value);
        msgSpan.append(elem);
        break;
      }
    }
  }
  if (modified) {
    const handlersEle = document.createElement('div');
    handlersEle.classList.add('handlers')
    extra.append(handlersEle);
  }


  const title = document.createElement("div");
  title.classList.add('message-title');
  title.append(name)

  const shadowBox = document.createElement("div");
  shadowBox.classList.add('shadow-box')
  shadowBox.append(title);
  msg.append(extra);
  shadowBox.append(msg);

  // Create message HTML element
  const message = document.createElement('li');
  message.append(shadowBox);

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
  // await transitioned(inactive);
  await asyncSleep(pushUpDuration * 1000);

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
}

async function handleCppFormater(data) {
  const refMessage = document.getElementById("message-" + data.ref_id);
  if(!refMessage) {
    console.warn("Unable to get message-" + data.ref_id + " . It might have faded out! Consider increase messageTimeout.")
    return;
  }
  
  refMessage.dataset.cppState = data.state;
  switch (data.state) {
    case 0 /* Success */: {
      const contentEle = refMessage.querySelector('.message-content>span');
      const commandListEle = document.createElement('pre');
      commandListEle.innerText = data.formatted_code;
      contentEle.innerHTML = "";
      contentEle.append(commandListEle);
      if (refMessage.parentElement.id == 'container-inactive')
        window.scrollTo(0, document.body.scrollHeight);
      break;
    }
    case 1 /* Error */: {
      const cppMsgEle = refMessage.querySelector('.extra > .handlers > .cpp > .state')
      cppMsgEle.innerText = "Error";
      break;
    }
    case 2 /* Formatting */: {
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
    case 3 /* Timeout */: {
      const cppMsgEle = refMessage.querySelector('.extra > .handlers > .cpp > .state')
      cppMsgEle.innerText = "Timeout";
    }
  }
}

async function handleCommandList(data) {
  const refMessage = document.getElementById("message-" + data.ref_id);
  refMessage.dataset.commandsState = data.state;
  switch (data.state) {
    case 0: {
      const contentEle = refMessage.querySelector('.message-content>span');
      const commandListText = document.createTextNode(data.commands);
      contentEle.innerHTML = "";
      contentEle.append(commandListText);
      if (refMessage.parentElement.id == 'container-inactive')
        window.scrollTo(0, document.body.scrollHeight);
      break;
    }
    case 1: {
      const commandListMsgEle = refMessage.querySelector('.extra > .handlers > .commands > .state')
      commandListMsgEle.innerText = "Error";
      break;
    }
    case 2: {
      refMessage.classList.add('modified');

      const handlersEle = refMessage.querySelector('.extra > .handlers');

      const handlerEle = document.createElement('div');
      handlerEle.classList.add('commands')

      const handlerNameEle = document.createElement('span');
      handlerNameEle.classList.add('name');
      handlerNameEle.innerText = 'commands';

      const handlerStateEle = document.createElement('span');
      handlerStateEle.classList.add('state');
      handlerStateEle.innerText = 'Querying';

      handlerEle.append(handlerNameEle, handlerStateEle);
      handlersEle.append(handlerEle);
      const handlers = (refMessage.dataset.handlers || "").split(' ');
      handlers.push('commands');
      refMessage.dataset.handlers = handlers.join(' ');
      break;
    }
    case 3: {
      const cppMsgEle = refMessage.querySelector('.extra > .handlers > .commands > .state')
      cppMsgEle.innerText = "Timeout";
    }
  }
}

async function handlePoke(data) {
  const refMessage = document.getElementById("message-" + data.ref_id);
  refMessage.dataset.pokeState = data.state;
  const contentEle = refMessage.querySelector('.message-content>span');

  const initElements = (state) => {
    refMessage.classList.add('modified');

    const handlersEle = refMessage.querySelector('.extra > .handlers');

    const handlerEle = document.createElement('div');
    handlerEle.classList.add('poke')

    const handlerNameEle = document.createElement('span');
    handlerNameEle.classList.add('name');
    handlerNameEle.innerText = 'poke';

    const handlerStateEle = document.createElement('span');
    handlerStateEle.classList.add('state');
    handlerStateEle.innerText = state;

    handlerEle.append(handlerNameEle, handlerStateEle);
    handlersEle.append(handlerEle);
    const handlers = (refMessage.dataset.handlers || "").split(' ');
    handlers.push('poke');
    refMessage.dataset.handlers = handlers.join(' ');
  };

  switch (data.state) {
    case 0: {
      const contentEle = refMessage.querySelector('.message-content>span');
      const commandListText = document.createTextNode("Poke completed!");
      contentEle.innerHTML = "";
      contentEle.append(commandListText);
      if (refMessage.parentElement.id == 'container-inactive')
        window.scrollTo(0, document.body.scrollHeight);
      break;
    }
    case 1: {
      const commandListMsgEle = refMessage.querySelector('.extra > .handlers > .poke > .state')
      commandListMsgEle.innerText = "Error";
      break;
    }
    case 2: {
      initElements("InProgress");
      const commandListText = document.createTextNode(data.from + " start" + " poking!");
      contentEle.innerHTML = "";
      contentEle.append(commandListText);

      var audio = new Audio('/media/kitten_meow.wav');
      audio.play();

      break;
    }
    case 3: {
      const cppMsgEle = refMessage.querySelector('.extra > .handlers > .poke > .state')
      cppMsgEle.innerText = "Timeout";
      const commandListText = document.createTextNode(data.from + " start" + " poking!");
      contentEle.innerHTML = "";
      contentEle.append(commandListText);

      break;
    }
    case 4: {
      initElements("Reject");
      const commandListText = document.createTextNode("Poke is in cooldown, next in " + Math.ceil(data.next_in_millis / 1000) + " seconds!");
      contentEle.innerHTML = "";
      contentEle.append(commandListText);
      break;
    }
  }
}


async function pushMessage(parsedMessage) {
  switch (parsedMessage.kind) {
    case '00000000-0000-0000-0000-000000000000': {
      await displayGenericMessage(parsedMessage.data);
      break;
    }
    case 'ee50b83b-9182-4e31-b33f-a6b94722cc85': {
      await handleCppFormater(parsedMessage.data);
      break;
    }
    case '1c66ecd2-663c-4e04-975f-a6593432e53c': {
      await handleCommandList(parsedMessage.data);
      break;
    }
    case 'c858e29c-c320-4de6-a227-013d8f7a90f0': {
      await handlePoke(parsedMessage.data);
      break;
    }
  }

  return;
}

const messageTimeout = 10.0;
container.dataset.messageCount = 0;
container.dataset.messageMax = 20;

const ws = new WebSocket('ws://localhost:8080');
const msgInQueue = [];

ws.onopen = () => {
  console.log('WebSocket connection opened');
}
ws.onmessage = (msg) => {
  console.log('Message: ', msg.data);
  const message = JSON.parse(msg.data);
  container.dataset.messageCount = parseInt(container.dataset.messageCount) + 1;
  msgInQueue.push([container.dataset.messageCount, message, message.data?.id]);
}

ws.onclose = (msg) => {
  console.log('Closed');
}

let messageIdx = 1;


setInterval(async () => {
  messageInfo = msgInQueue.at(0);
  if(!messageInfo) return;
  const [idx, message, id]  = messageInfo;
  if (messageIdx != idx) return;
  
  if(message.kind == '00000000-0000-0000-0000-000000000000') { 
    messageIdx++;
    await pushMessage(message);
    setTimeout(async() => {
      const refMessage = document.getElementById("message-" + id);
      // Fading out message
      refMessage.classList.add('fadeOut');
      // Wait for message fade out animation to finish
      await asyncSleep(fadeOutDuration * 1000);
      // Remove message from the DOM
      refMessage.remove()
    }, messageTimeout * 1000);
    
    msgInQueue.shift();
  } else {
    messageIdx++;
    if(message.data.ref_id) {
      const refMessage = document.getElementById("message-" + message.data.ref_id);
      // if(refMessage.classList.contains('fadeIn')); {
      //   await asyncSleep(fadeInDuration * 1000)
      // }
      await pushMessage(message, messageTimeout);
    }
    msgInQueue.shift();
  }
}, 100);
